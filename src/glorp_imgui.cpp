#include "glorp_imgui.hpp"
#include "glorp_swap_chain.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <cmath>
#include <numeric>

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
    init_info.MSAASamples = m_glorpDevice.getSupportedSampleCount();
    ImGui_ImplVulkan_Init(&init_info);
}

void GlorpImgui::drawUI(FrameInfo &frameInfo) {
    updateFPS();
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    defaultWindow(frameInfo);

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, frameInfo.commandBuffer);
}

void GlorpImgui::updateFPS() {
    auto currentTime = std::chrono::steady_clock::now();

    std::chrono::duration<float> duration = currentTime - startTime;
    m_elapsedTime = duration.count();

    m_frameCount++;

    if (m_elapsedTime >= TIME_WINDOW) {
        float fps = m_frameCount / TIME_WINDOW;

        m_roundedAverageFPS = static_cast<int>(std::round(fps));

        m_frameCount = 0;
        m_elapsedTime = 0.0f;
        startTime = std::chrono::steady_clock::now();
    }
}

void GlorpImgui::defaultWindow(FrameInfo &frameInfo) {
    ImGui::Begin("Debug");
    ImGui::Text("Frame time (ms): %f",frameInfo.frameTime * 1000);

    ImGui::Text("Frames Per Second: %d", m_roundedAverageFPS);

    if(ImGui::CollapsingHeader("Light Control")) {
        ImGui::SliderFloat("Brightness", &m_lightBrightness, 0.0f, 1.0f);
        ImGui::SliderFloat("Rotation Multiplier", &m_rotationMultiplier, 0.0f, 10.f);
        ImGui::SliderFloat("Vertical position", &lightPosition, -5.f, 5.f);
    }
    if(ImGui::CollapsingHeader("Texture Control")) {
        ImGui::Checkbox("Use Albedo", &useAlbedoMap);
        ImGui::Checkbox("Use Normal", &useNormalMap);
        ImGui::Checkbox("Use Emmisive", &useEmissiveMap);
        ImGui::Checkbox("Use AO", &useAOMap);
    }
    if(ImGui::CollapsingHeader("Fog Control")) {
        ImGui::Checkbox("Enable Fog", &fogEnabled);
        ImGui::DragFloat("Fog Start", &fogStart);
        ImGui::DragFloat("Fog End", &fogEnd);
        
        ImGui::ColorPicker3("Fog Color", (float *) &fogColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoSidePreview);
    }

    ImGui::End();
}
}
