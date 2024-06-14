#pragma once

#include "glorp_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace Glorp {
class GlorpModel {
    public:
        struct Vertex {
          glm::vec2 position;
          glm::vec3 color;

          static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(); 
          static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(); 
        };
        
        GlorpModel(GlorpDevice &device, const std::vector<Vertex> &vertices);
        ~GlorpModel();

        GlorpModel(const GlorpModel&) = delete;
        GlorpModel &operator=(const GlorpModel &) = delete;
        

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

    private:
        GlorpDevice& m_glorpDevice;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;
};
}