#pragma once

#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"
#include "glorp_descriptors.hpp"

#include <memory>

namespace Glorp {

class GlorpImgui {
    public:
        GlorpImgui(GlorpDevice &device, VkRenderPass renderPass, GlorpWindow &window);
        ~GlorpImgui();

        void drawUI(FrameInfo &frameInfo);
    private:
        void initImgui(VkRenderPass renderPass);
    private:
        GlorpDevice &m_glorpDevice;
        GlorpWindow &m_glorpWindow;
        std::unique_ptr<GlorpDescriptorPool> imguiPool {};
};
}