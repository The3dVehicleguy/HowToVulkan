// VulkanApp.cpp
#include "VulkanApp.h"

#include <SFML/Graphics.hpp>
#include <vulkan/vulkan.h>
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <optional>
#include <cassert>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
#include <ktx.h>
#include <ktxvulkan.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "InstanceWrapper.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Swapchain.h"
#include "CommandPool.h"
#include "Descriptor.h"
#include "TextureImage.h"
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

int VulkanApp::run()
{
    // Initialize volk loader then create an instance via the RAII wrapper.
    volkInitialize();
    InstanceWrapper inst("How to Vulkan");
    VkInstance instance = inst.get();

    // Choose a physical device via helper
    uint32_t deviceIndex{ 0 };
    if (argc_ > 1) {
        deviceIndex = std::stoi(argv_[1]);
    }
    PhysicalDevice physHelper;
    VkPhysicalDevice physical = physHelper.choose(instance, deviceIndex);
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
    VkDevice device = logicalHelper.create(physical, queueFamily);
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
    // More verbose check for surface creation to help diagnose runtime failures.
    bool surfaceCreated = window.createVulkanSurface(instance, surface);
    if (!surfaceCreated) {
        std::cerr << "createVulkanSurface returned false\n";
        chk(false); // will print generic message and exit
    }
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

    // Mesh data
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::cerr << "Loading mesh assets/suzanne.obj...\n";
    bool meshLoaded = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, "assets/suzanne.obj");
    if (!meshLoaded) {
        std::cerr << "tinyobj::LoadObj failed\n";
        chk(false);
    }
    const VkDeviceSize indexCount{ shapes[0].mesh.indices.size() };
    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};

    for (auto& index : shapes[0].mesh.indices) {
        Vertex v{
            .pos = { attrib.vertices[index.vertex_index * 3], -attrib.vertices[index.vertex_index * 3 + 1], attrib.vertices[index.vertex_index * 3 + 2] },
            .normal = { attrib.normals[index.normal_index * 3], -attrib.normals[index.normal_index * 3 + 1], attrib.normals[index.normal_index * 3 + 2] },
            .uv = { attrib.texcoords[index.texcoord_index * 2], 1.0f - attrib.texcoords[index.texcoord_index * 2 + 1] }
        };
        vertices.push_back(v);
        indices.push_back(indices.size());
    }
    VkDeviceSize vBufSize{ sizeof(Vertex) * vertices.size() };
    VkDeviceSize iBufSize{ sizeof(uint16_t) * indices.size() };
    VkBuffer vBuffer{ VK_NULL_HANDLE };
    VmaAllocation vBufferAllocation{ VK_NULL_HANDLE };
    VkBufferCreateInfo bufferCI{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = vBufSize + iBufSize, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT };
    VmaAllocationCreateInfo bufferAllocCI{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
    chk(vmaCreateBuffer(allocator, &bufferCI, &bufferAllocCI, &vBuffer, &vBufferAllocation, nullptr));
    void* bufferPtr{ nullptr };
    vmaMapMemory(allocator, vBufferAllocation, &bufferPtr);
    memcpy(bufferPtr, vertices.data(), vBufSize);
    memcpy(((char*)bufferPtr) + vBufSize, indices.data(), iBufSize);
    vmaUnmapMemory(allocator, vBufferAllocation);

    // Shader data buffers
    std::array<ShaderDataBuffer, VulkanApp::maxFramesInFlight> shaderDataBuffers{};
    for (auto i = 0; i < VulkanApp::maxFramesInFlight; i++) {
        VkBufferCreateInfo uBufferCI{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = sizeof(ShaderData), .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
        VmaAllocationCreateInfo uBufferAllocCI{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
        chk(vmaCreateBuffer(allocator, &uBufferCI, &uBufferAllocCI, &shaderDataBuffers[i].buffer, &shaderDataBuffers[i].allocation, nullptr));
        vmaMapMemory(allocator, shaderDataBuffers[i].allocation, &shaderDataBuffers[i].mapped);
        VkBufferDeviceAddressInfo uBufferBdaInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = shaderDataBuffers[i].buffer };
        shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(device, &uBufferBdaInfo);
    }

    // Sync objects
    std::array<VkFence, VulkanApp::maxFramesInFlight> fences{};
    std::array<VkSemaphore, VulkanApp::maxFramesInFlight> presentSemaphores{};
    std::vector<VkSemaphore> renderSemaphores;
    VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    for (auto i = 0; i < VulkanApp::maxFramesInFlight; i++) {
        chk(vkCreateFence(device, &fenceCI, nullptr, &fences[i]));
        chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentSemaphores[i]));
    }
    renderSemaphores.resize(swapchainImages.size());
    for (auto& semaphore : renderSemaphores) {
        chk(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
    }

    // Command pool (use RAII helper)
    CommandPool cmdPoolHelper(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    std::array<VkCommandBuffer, VulkanApp::maxFramesInFlight> commandBuffers{};
    auto allocated = cmdPoolHelper.allocate(device, VulkanApp::maxFramesInFlight);
    for (size_t i = 0; i < allocated.size() && i < commandBuffers.size(); ++i) commandBuffers[i] = allocated[i];

    // Texture images (load via helper)
    std::array<Texture, 3> textures{};
    std::vector<VkDescriptorImageInfo> textureDescriptors{};
    TextureImage texLoader;
    for (size_t i = 0; i < textures.size(); ++i) {
        std::string filename = "assets/suzanne" + std::to_string(i) + ".ktx";
    textures[i] = texLoader.load(device, allocator, cmdPoolHelper.getPool(), queue, filename);
        if (textures[i].image == VK_NULL_HANDLE) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            chk(VK_ERROR_INITIALIZATION_FAILED);
        }
        textureDescriptors.push_back({ .sampler = textures[i].sampler, .imageView = textures[i].view, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });
    }

    // Descriptor (indexing) - use helper to create layout, pool and allocate/update set
    Descriptor descHelper;
    VkDescriptorSetLayout descriptorSetLayoutTex = descHelper.createLayout(device, static_cast<uint32_t>(textures.size()));
    VkDescriptorPool descriptorPool = descHelper.createPool(device, static_cast<uint32_t>(textures.size()));
    if (descriptorPool == VK_NULL_HANDLE) {
        std::cerr << "Failed to create descriptor pool" << std::endl;
        chk(VK_ERROR_INITIALIZATION_FAILED);
    }
    VkDescriptorSet descriptorSetTex = descHelper.allocateAndWrite(device, descriptorPool, descriptorSetLayoutTex, textureDescriptors);
    if (descriptorSetTex == VK_NULL_HANDLE) {
        std::cerr << "Failed to allocate descriptor set" << std::endl;
        chk(VK_ERROR_INITIALIZATION_FAILED);
    }

    // Initialize Slang shader compiler
    Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;
    slang::createGlobalSession(slangGlobalSession.writeRef());
    auto slangTargets{ std::to_array<slang::TargetDesc>({ {.format{SLANG_SPIRV}, .profile{slangGlobalSession->findProfile("spirv_1_4")} } }) };
    auto slangOptions{ std::to_array<slang::CompilerOptionEntry>({ { slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1} } }) };
    slang::SessionDesc slangSessionDesc{ .targets{slangTargets.data()}, .targetCount{SlangInt(slangTargets.size())}, .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, .compilerOptionEntries{slangOptions.data()}, .compilerOptionEntryCount{uint32_t(slangOptions.size())} };

    // Load shader
    Slang::ComPtr<slang::ISession> slangSession;
    slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
    Slang::ComPtr<slang::IModule> slangModule{ slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr) };
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());
    VkShaderModuleCreateInfo shaderModuleCI{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = spirv->getBufferSize(), .pCode = (uint32_t*)spirv->getBufferPointer() };
    VkShaderModule shaderModule{};
    vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule);

    // Pipeline layout (push constant for device address)
    VkPushConstantRange pushConstantRange{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress) };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &descriptorSetLayoutTex, .pushConstantRangeCount = 1, .pPushConstantRanges = &pushConstantRange };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    chk(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

    // Vertex input description
    VkVertexInputBindingDescription vertexBinding{ .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    std::vector<VkVertexInputAttributeDescription> vertexAttributes{
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal) },
        { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv) },
    };

    // Use Pipeline wrapper to create the graphics pipeline
    Pipeline pipelineHelper;
    VkPipeline pipeline = pipelineHelper.createGraphics(device, pipelineLayout, shaderModule, vertexBinding, vertexAttributes, imageFormat, depthFormat);
    if (pipeline == VK_NULL_HANDLE) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        chk(VK_ERROR_INITIALIZATION_FAILED);
    }

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
    ctx.shaderDataBuffers = &shaderDataBuffers;
    ctx.commandBuffers = &commandBuffers;
    ctx.fences = &fences;
    ctx.presentSemaphores = &presentSemaphores;
    ctx.renderSemaphores = &renderSemaphores;
    ctx.surfaceCaps = &surfaceCaps;

    int rendererExit = renderer.run(ctx);
    if (rendererExit != 0) {
        return rendererExit;
    }

    // Tear down
    chk(vkDeviceWaitIdle(device));
    for (auto i = 0; i < VulkanApp::maxFramesInFlight; i++) {
        vkDestroyFence(device, fences[i], nullptr);
        vkDestroySemaphore(device, presentSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderSemaphores[i], nullptr);
        vmaUnmapMemory(allocator, shaderDataBuffers[i].allocation);
        vmaDestroyBuffer(allocator, shaderDataBuffers[i].buffer, shaderDataBuffers[i].allocation);
    }
    // Swapchain helper owns swapchain images, image views and depth image.
    swapHelper.destroy(device, allocator);
    vmaDestroyBuffer(allocator, vBuffer, vBufferAllocation);
    for (auto i = 0; i < textures.size(); i++) {
        vkDestroyImageView(device, textures[i].view, nullptr);
        vkDestroySampler(device, textures[i].sampler, nullptr);
        vmaDestroyImage(allocator, textures[i].image, textures[i].allocation);
    }
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutTex, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    // swapHelper.destroy already cleaned up the swapchain
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // Command pool will be destroyed automatically by CommandPool's destructor.
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    return 0;
}
