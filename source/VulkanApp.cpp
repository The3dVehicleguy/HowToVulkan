// VulkanApp.cpp
#include "VulkanApp.h"

#include <SFML/Graphics.hpp>
#include <vulkan/vulkan.h>
#define VOLK_IMPLEMENTATION
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <volk/volk.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "slang/slang-com-ptr.h"
#include "slang/slang.h"
#include <ktx.h>
#include <ktxvulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "CommandPool.h"
#include "Descriptor.h"
#include "InstanceWrapper.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "Renderer.h"

static inline void chk(VkResult result) {
    if (result != VK_SUCCESS) {
        std::cerr << "Vulkan call returned an error (" << result << ")\n";
        exit(result);
    }
}
static inline void chk(bool result) {
    if (!result) {
        std::cerr << "Call returned an error\n";
        exit(EXIT_FAILURE);
    }
}

VulkanApp::VulkanApp(int argc, char* argv[])
    : argc_(argc), argv_(argc ? argv : nullptr) {}

VulkanApp::~VulkanApp() = default;

int VulkanApp::Run()
{
    // Initialize volk loader then create an instance via the RAII wrapper.
    volkInitialize();
    InstanceWrapper inst("How to Vulkan");
    VkInstance instance = inst.Get();

    // Choose a physical device via helper
    uint32_t deviceIndex{ 0 };
    if (argc_ > 1) {
        deviceIndex = std::stoi(argv_[1]);
    }
    PhysicalDevice physHelper;
    VkPhysicalDevice physical = physHelper.Choose(instance, deviceIndex);
    VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(physical, &deviceProperties);
    std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";

    // Find a queue family for graphics
    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{ 0 };
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily = i;
            break;
        }
    }

    // Create logical device via helper
    LogicalDevice logicalHelper;
    VkDevice device = logicalHelper.Create(physical, queueFamily);
    VkQueue queue{ VK_NULL_HANDLE };
    vkGetDeviceQueue(device, queueFamily, 0, &queue);

    // VMA
    VmaVulkanFunctions vkFunctions{ .vkGetInstanceProcAddr = vkGetInstanceProcAddr, .vkGetDeviceProcAddr = vkGetDeviceProcAddr, .vkCreateImage = vkCreateImage };
    VmaAllocatorCreateInfo allocatorCI{ .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, .physicalDevice = physical, .device = device, .pVulkanFunctions = &vkFunctions, .instance = instance };
    VmaAllocator allocator{ VK_NULL_HANDLE };
    chk(vmaCreateAllocator(&allocatorCI, &allocator));

    // Window and surface
    auto window = sf::RenderWindow(sf::VideoMode({ 1280, 720u }), "How to Vulkan");
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    chk(window.createVulkanSurface(instance, surface));
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &surfaceCaps));

    // Swap chain (use helper)
    Swapchain swapHelper;
    std::cerr << "VulkanApp: calling swapHelper.create(...)\n";
    VkSwapchainKHR swapchain = swapHelper.create(physical, device, surface, queueFamily, allocator);
    auto& swapchainImages = swapHelper.images();
    auto& swapchainImageViews = swapHelper.imageViews();
    const VkFormat imageFormat = swapHelper.getImageFormat();
    const VkFormat depthFormat = swapHelper.getDepthFormat();
    uint32_t imageCount = static_cast<uint32_t>(swapchainImages.size());

    // Command pool needed for texture uploads and transient work (create early)
    CommandPool cmdPoolHelper(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Assets: load model and textures via AssetManager
    VmaAllocationCreateInfo bufferAllocCI{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
    AssetManager assetMgr;
    std::vector<std::string> texFiles = { "assets/suzanne0.ktx", "assets/suzanne1.ktx", "assets/suzanne2.ktx" };
    chk(assetMgr.AddAsset(device, allocator, cmdPoolHelper.GetPool(), queue, "assets/suzanne.obj", texFiles, bufferAllocCI));
    const Asset& asset = assetMgr.GetAsset(0);
    VkBuffer vBuffer = asset.GetModelBuffer();
    VkDeviceSize vBufSize = asset.GetModelVertexSize();
    VkDeviceSize iBufSize = asset.GetModelIndexSize();
    VkDeviceSize indexCount = asset.GetModelIndexCount();

    // Shader data buffers (per-frame) managed by UniformBufferSet RAII helper
    UniformBufferSet uniformBuffers(allocator, device);

    // Sync objects: use owning RAII wrapper. Caller must destroy before device teardown.
    // SyncObjects is a non-templated class: pass framesInFlight, device and swapchain image count.
    SyncObjects sync(static_cast<uint32_t>(VulkanApp::maxFramesInFlight), device, static_cast<uint32_t>(swapchainImages.size()));

    std::array<VkCommandBuffer, VulkanApp::maxFramesInFlight> commandBuffers{};
    auto allocated = cmdPoolHelper.Allocate(device, VulkanApp::maxFramesInFlight);
    for (size_t i = 0; i < allocated.size() && i < commandBuffers.size(); ++i) commandBuffers[i] = allocated[i];

    // Textures are owned by the Asset instances (loaded inside Asset)

    // Descriptor (indexing) - use helper to create layout, pool and allocate/update set
    // Descriptor set(s) are created per-asset by AssetManager. Use the first asset's set here.
    VkDescriptorSetLayout descriptorSetLayoutTex = asset.GetDescriptorLayout();
    VkDescriptorSet descriptorSetTex = asset.GetDescriptorSet();

    // Initialize Slang shader compiler
    Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;
    slang::createGlobalSession(slangGlobalSession.writeRef());
    auto slangTargets{ std::to_array<slang::TargetDesc>({ {.format{SLANG_SPIRV}, .profile{slangGlobalSession->findProfile("spirv_1_4")} } }) };
    auto slangOptions{ std::to_array<slang::CompilerOptionEntry>({ {.name = slang::CompilerOptionName::EmitSpirvDirectly, .value = {slang::CompilerOptionValueKind::Int, 1} } }) };
    slang::SessionDesc slangSessionDesc{ .targets{slangTargets.data()}, .targetCount{SlangInt(slangTargets.size())}, .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, .compilerOptionEntries{slangOptions.data()}, .compilerOptionEntryCount{uint32_t(slangOptions.size())} };

    // Load shader
    Slang::ComPtr<slang::ISession> slangSession;
    slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
    Slang::ComPtr<slang::IModule> slangModule{ slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr) };
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());
    // Create ShaderManager owning shader modules for the pipeline stages.
    ShaderManager shaderManager(device, spirv->getBufferPointer(), spirv->getBufferSize());

    // Pipeline layout (push constant for device address)
    VkPushConstantRange pushConstantRange{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress) };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &descriptorSetLayoutTex, .pushConstantRangeCount = 1, .pPushConstantRanges = &pushConstantRange };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    chk(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

    // Vertex input description (use Asset/Model helpers)
    VkVertexInputBindingDescription vertexBinding = asset.GetVertexBindingDescription();
    std::vector<VkVertexInputAttributeDescription> vertexAttributes = asset.GetVertexAttributeDescriptions();

    // Use Pipeline wrapper to create the graphics pipeline via the new descriptor struct
    Pipeline pipelineHelper;
    Pipeline::GraphicsCreateInfo pci{};
    pci.device = device;
    pci.layout = pipelineLayout;
    pci.shaderManager = &shaderManager;
    pci.vertexBinding = vertexBinding;
    pci.vertexAttributes = vertexAttributes;
    pci.colorFormat = imageFormat;
    pci.depthFormat = depthFormat;
    VkPipeline pipeline = pipelineHelper.createGraphics(pci);
    if (pipeline == VK_NULL_HANDLE) {
        std::cerr << "Failed to create graphics pipeline" << '\n';
        chk(VK_ERROR_INITIALIZATION_FAILED);
    }
    OwnedPipeline ownedPipeline(device, pipeline, pipelineLayout);

    // Move render loop into Renderer class for cleaner separation of
    // responsibilities. The renderer operates on the Vulkan objects
    // created above and will return when the window is closed.
    Renderer renderer;
    // Build a RenderContext and hand it to the renderer.
    RenderContext ctx{};
    ctx.window = &window;
    ctx.physical = physical;
    ctx.surface = surface;
    ctx.queueFamily = queueFamily;
    ctx.device = device;
    ctx.queue = queue;
    ctx.allocator = allocator;
    ctx.swapchain = &swapHelper;
    ctx.pipeline = pipeline;
    ctx.pipelineLayout = pipelineLayout;
    ctx.descriptorSetTex = descriptorSetTex;
    ctx.vBuffer = vBuffer;
    ctx.vBufSize = vBufSize;
    ctx.indexCount = indexCount;
    // Build renderable list from assets (owned by VulkanApp stack here)
    std::vector<RenderContext::Renderable> renderables;
    renderables.push_back({ asset.GetDescriptorSet(), asset.GetModelBuffer(), asset.GetModelVertexSize(), asset.GetModelIndexSize(), asset.GetModelIndexCount() });
    ctx.renderables = &renderables;
    ctx.shaderDataBuffers = &uniformBuffers.buffers();
    ctx.commandBuffers = &commandBuffers;
    ctx.fences = &sync.fences();
    ctx.presentSemaphores = &sync.presentSemaphores();
    ctx.renderSemaphores = &sync.renderSemaphores();
    ctx.surfaceCaps = &surfaceCaps;

    int rendererExit = renderer.Run(ctx);
    if (rendererExit != 0) {
        return rendererExit;
    }

    // Tear down (explicit destruction ordering)
    chk(vkDeviceWaitIdle(device));
    // Destroy per-frame uniform buffers
    uniformBuffers.destroy(device);
    // Destroy non-device-local resources in correct order:
    // 1) Sync objects
    sync.destroy(device);
    // 2) Destroy assets (includes textures and per-asset descriptors and model buffers)
    assetMgr.DestroyAll(device, allocator);
    // 4) Destroy swapchain resources
    swapHelper.Destroy(device, allocator);
    // 5) Destroy pipeline and its layout
    ownedPipeline.destroy(device);
    // 6) Destroy surface
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // 7) Destroy command pool
    cmdPoolHelper.Destroy();
    // Vertex/index buffers are owned by assets; they've been destroyed by assetMgr.destroyAll
    // 8) Destroy allocator and device
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    // Instance is destroyed automatically by InstanceWrapper destructor

    return 0;
}
