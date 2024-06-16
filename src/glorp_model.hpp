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
          glm::vec3 position;
          glm::vec3 color;

          static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(); 
          static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(); 
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
        };
        
        GlorpModel(GlorpDevice &device, const GlorpModel::Builder &builder);
        ~GlorpModel();

        GlorpModel(const GlorpModel&) = delete;
        GlorpModel &operator=(const GlorpModel &) = delete;
        

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);


    private:
        GlorpDevice& m_glorpDevice;

        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;
        uint32_t m_indexCount;
};
}