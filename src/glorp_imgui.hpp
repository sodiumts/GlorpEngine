#pragma once

#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"
#include "glorp_descriptors.hpp"

#include <memory>
#include <chrono>

namespace Glorp {

class GlorpImgui {
    public:
        GlorpImgui(GlorpDevice &device, VkRenderPass renderPass, GlorpWindow &window);
        ~GlorpImgui();

        void drawUI(FrameInfo &frameInfo);
        float getLightIntensity() { return m_lightBrightness; }
        float getRotationMultiplier() { return m_rotationMultiplier; }

        bool useNormalMap{true};
        bool useAlbedoMap{true};
        bool useEmissiveMap{true};
        bool useAOMap{true};
    private:
        void initImgui(VkRenderPass renderPass);
        void defaultWindow(FrameInfo &frameInfo);
        void updateFPS();
    private:
        GlorpDevice &m_glorpDevice;
        GlorpWindow &m_glorpWindow;
        std::unique_ptr<GlorpDescriptorPool> imguiPool {};

        float m_lightBrightness = .5f;
        float m_rotationMultiplier = 1.f;

        int m_roundedAverageFPS = 0;
        const float TIME_WINDOW = 0.5f;
        int m_frameCount = 0;
        float m_elapsedTime = 0.0f;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
};
}
