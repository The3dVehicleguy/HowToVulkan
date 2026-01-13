// Renderer.cpp
#include "Renderer.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <vulkan/vulkan.h>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Swapchain.h"
#include <array>
#include <cstring>
#include <cstdlib>
#include <optional>

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

int Renderer::run(const RenderContext& ctx)

{
    // Copied render loop from previous monolithic implementation and
    // refactored to operate on the references passed in by the app.
    sf::Clock clock;
    uint32_t imageIndex{ 0 };
    uint32_t frameIndex{ 0 };
    ShaderData shaderData{};
    glm::vec3 camPos{ 0.0f, 0.0f, -6.0f };
    glm::vec3 objectRotations[3]{};
    sf::Vector2i lastMousePos{};

    // Local aliases to simplify access to context members
    auto& window = *ctx.window;
    auto& device = ctx.device;
    auto& queue = ctx.queue;
    auto& allocator = ctx.allocator;
    auto& swapHelper = *ctx.swapchain;
    auto& pipeline = ctx.pipeline;
    auto& pipelineLayout = ctx.pipelineLayout;
    auto& descriptorSetTex = ctx.descriptorSetTex;
    auto& vBuffer = ctx.vBuffer;
    auto vBufSize = ctx.vBufSize;
    auto indexCount = ctx.indexCount;
    auto& shaderDataBuffers = *ctx.shaderDataBuffers;
    auto& commandBuffers = *ctx.commandBuffers;
    auto& fences = *ctx.fences;
    auto& presentSemaphores = *ctx.presentSemaphores;
    auto& renderSemaphores = *ctx.renderSemaphores;
    auto& surfaceCaps = *ctx.surfaceCaps;
    auto physical = ctx.physical;
    auto queueFamily = ctx.queueFamily;

    while (window.isOpen()) {

        // Sync
        chk(vkWaitForFences(device, 1, &fences[frameIndex], true, UINT64_MAX));
        chk(vkResetFences(device, 1, &fences[frameIndex]));
        VkSwapchainKHR swapchain = swapHelper.get();
        auto &swapchainImages = swapHelper.images();
        auto &swapchainImageViews = swapHelper.imageViews();
        VkImageView depthImageView = swapHelper.getDepthView();
        vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);

        // Update shader data
    shaderData.projection = glm::perspective(glm::radians(45.0f), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 32.0f);
        shaderData.view = glm::translate(glm::mat4(1.0f), camPos);
        for (auto i = 0; i < 3; i++) {
            auto instancePos = glm::vec3((float)(i - 1) * 3.0f, 0.0f, 0.0f);
            shaderData.model[i] = glm::translate(glm::mat4(1.0f), instancePos) * glm::mat4_cast(glm::quat(objectRotations[i]));
        }
        memcpy(shaderDataBuffers[frameIndex].mapped, &shaderData, sizeof(ShaderData));

        // Build command buffer
    auto cb = commandBuffers[frameIndex];
        vkResetCommandBuffer(cb, 0);
        VkCommandBufferBeginInfo cbBI { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
        vkBeginCommandBuffer(cb, &cbBI);
        std::array<VkImageMemoryBarrier2, 2> outputBarriers{
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = swapchainImages[imageIndex],
                .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
            },
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = swapHelper.getDepthImage(),
                .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1, .layerCount = 1 }
            }
        };
        VkDependencyInfo barrierDependencyInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 2, .pImageMemoryBarriers = outputBarriers.data() };
        vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
        VkRenderingAttachmentInfo colorAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = swapchainImageViews[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{ 0.0f, 0.0f, 0.0f, 1.0f }}
        };
        VkRenderingAttachmentInfo depthAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = depthImageView,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {.depthStencil = {1.0f,  0}}
        };
        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{.extent{.width = window.getSize().x, .height = window.getSize().y }},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttachmentInfo
        };
        vkCmdBeginRendering(cb, &renderingInfo);
        VkViewport vp{ .width = static_cast<float>(window.getSize().x), .height = static_cast<float>(window.getSize().y), .minDepth = 0.0f, .maxDepth = 1.0f};
        vkCmdSetViewport(cb, 0, 1, &vp);
        VkRect2D scissor{ .extent{ .width = window.getSize().x, .height = window.getSize().y } };
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdSetScissor(cb, 0, 1, &scissor);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetTex, 0, nullptr);
        VkDeviceSize vOffset{ 0 };
        vkCmdBindVertexBuffers(cb, 0, 1, &vBuffer, &vOffset);
        vkCmdBindIndexBuffer(cb, vBuffer, vBufSize, VK_INDEX_TYPE_UINT16);
    vkCmdPushConstants(cb, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &shaderDataBuffers[frameIndex].deviceAddress);
    vkCmdDrawIndexed(cb, indexCount, 3, 0, 0, 0);
        vkCmdEndRendering(cb);
        VkImageMemoryBarrier2 barrierPresent{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = swapchainImages[imageIndex],
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
        };
        VkDependencyInfo barrierPresentDependencyInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrierPresent };
        vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
        vkEndCommandBuffer(cb);

        // Submit to graphics queue
        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &presentSemaphores[frameIndex],
            .pWaitDstStageMask = &waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &cb,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderSemaphores[imageIndex],
        };
        chk(vkQueueSubmit(queue, 1, &submitInfo, fences[frameIndex]));
        frameIndex = (frameIndex + 1) % VulkanApp::maxFramesInFlight;
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderSemaphores[imageIndex],
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIndex
        };
        chk(vkQueuePresentKHR(queue, &presentInfo));

        // Event polling
        sf::Time elapsed = clock.restart();
    while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    auto delta = lastMousePos - mouseMoved->position;
                    objectRotations[shaderData.selected].x += (float)delta.y * 0.0005f * (float)elapsed.asMilliseconds();
                    objectRotations[shaderData.selected].y -= (float)delta.x * 0.0005f * (float)elapsed.asMilliseconds();
                }
                lastMousePos = mouseMoved->position;
            }
            if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                camPos.z += (float)mouseWheelScrolled->delta * 0.025f * (float)elapsed.asMilliseconds();
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Add) {
                    shaderData.selected = (shaderData.selected < 2) ? shaderData.selected + 1 : 0;
                }
                if (keyPressed->code == sf::Keyboard::Key::Subtract) {
                    shaderData.selected = (shaderData.selected > 0) ? shaderData.selected - 1 : 2;
                }
            }

            // Window resize - recreate swapchain and depth image
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                vkDeviceWaitIdle(device);
                chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, ctx.surface, &surfaceCaps));
                // Recreate swapchain via helper (it will destroy/create its images, views and depth image)
                vkDeviceWaitIdle(device);
                swapHelper.recreate(physical, device, ctx.surface, queueFamily, allocator);
                // After recreation, refresh local references to images/views
                // (they are fetched each frame at the top of the loop)
            }
        }
    }

    return 0;
}
