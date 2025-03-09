#pragma once

#include "glorp_device.hpp"


#include "tiny_gltf.h"

namespace Glorp {
class Glorp3DTexture {
    public:
        Glorp3DTexture(GlorpDevice &device, const std::string &texture3D, int depth);
        ~Glorp3DTexture();

        Glorp3DTexture (const Glorp3DTexture&) = delete;
        Glorp3DTexture& operator= (const Glorp3DTexture&) = delete;

        VkSampler getSampler() { return m_sampler; }

        VkImageView getImageView() { return m_imageView; }
        VkImageLayout getImageLayout() { return m_imageLayout; }
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void createSampler();
        void createImageView();

        void create3DTexture(const std::string &texture3D, int depth);
    private:

        int m_height, m_width;
        GlorpDevice& m_device;
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkFormat m_imageFormat;
        VkImageLayout m_imageLayout;
};
}
