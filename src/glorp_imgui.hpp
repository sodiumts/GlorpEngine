#pragma once

#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"
#include "glorp_descriptors.hpp"

#include <memory>
#include <vector>

namespace Glorp {

class GlorpImgui {
    public:
        GlorpImgui(GlorpDevice &device, VkRenderPass renderPass, GlorpWindow &window);
        ~GlorpImgui();

        void drawUI(FrameInfo &frameInfo);
        float getLightIntensity() { return m_lightBrightness; }
        float getRotationMultiplier() { return m_rotationMultiplier; }
    private:
        void initImgui(VkRenderPass renderPass);
        void defaultWindow(FrameInfo &frameInfo);
    private:
        GlorpDevice &m_glorpDevice;
        GlorpWindow &m_glorpWindow;
        std::unique_ptr<GlorpDescriptorPool> imguiPool {};

        float m_lightBrightness = .5f;
        float m_rotationMultiplier = 1.f;
        int m_maxFpsSamples = 2000;
        std::vector<float> fpsSamples;

};
}