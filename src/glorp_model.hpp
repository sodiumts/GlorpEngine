#pragma once

#include "glorp_device.hpp"
#include "glorp_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

#include "tiny_gltf.h"


namespace Glorp {
class GlorpModel {
    public:
        struct Vertex {
          glm::vec3 position {};
          glm::vec3 color {};
          glm::vec3 normal {};
          glm::vec2 uv {};
          glm::vec3 tangent {};
          glm::vec3 bitangent {};

          static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
          static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

          bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
          }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModelFromGLTF(tinygltf::Model &model);
        };

        GlorpModel(GlorpDevice &device, const GlorpModel::Builder &builder);
        ~GlorpModel();

        GlorpModel(const GlorpModel&) = delete;
        GlorpModel &operator=(const GlorpModel &) = delete;

        static std::unique_ptr<GlorpModel> createModelFromGLTF(GlorpDevice &device, tinygltf::Model &model);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);


    private:
        GlorpDevice& m_glorpDevice;

        std::unique_ptr<GlorpBuffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        bool m_hasIndexBuffer = false;
        std::unique_ptr<GlorpBuffer> m_indexBuffer;
        uint32_t m_indexCount;
};
}
