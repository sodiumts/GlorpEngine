#include "glorp_imgui.hpp"
#include "glorp_swap_chain.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Glorp {
GlorpImgui::GlorpImgui(GlorpDevice &device, VkRenderPass renderPass, GlorpWindow &window) : m_glorpDevice(device), m_glorpWindow(window) {
    initImgui(renderPass);
}

GlorpImgui::~GlorpImgui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GlorpImgui::initImgui(VkRenderPass renderPass) {
    imguiPool = GlorpDescriptorPool::Builder(m_glorpDevice)
        .setMaxSets(1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .build();
        
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(m_glorpWindow.getGLFWwindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_glorpDevice.instance();
    init_info.PhysicalDevice = m_glorpDevice.physicalDevice();
    init_info.Device = m_glorpDevice.device();
    init_info.QueueFamily = m_glorpDevice.findPhysicalQueueFamilies().graphicsFamily;
    init_info.Queue = m_glorpDevice.graphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool->getRawPool();
    init_info.RenderPass = renderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = GlorpSwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = GlorpSwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);
}

void GlorpImgui::drawUI(FrameInfo &frameInfo) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, frameInfo.commandBuffer);
}
}
