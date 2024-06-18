#pragma once

#include "glorp_camera.hpp"

#include <vulkan/vulkan.h>

namespace Glorp {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        GlorpCamera &camera;
        VkDescriptorSet globalDescriptorSet;
    };
}