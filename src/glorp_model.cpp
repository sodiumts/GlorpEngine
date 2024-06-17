#include "glorp_model.hpp"

#include "glorp_utils.hpp"

#include <cassert>
#include <cstring>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
template <>
struct hash<Glorp::GlorpModel::Vertex> {
    size_t operator()(Glorp::GlorpModel::Vertex const &vertex) const {
        size_t seed = 0;
        Glorp::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};
}

namespace Glorp {
GlorpModel::GlorpModel(GlorpDevice &device,const GlorpModel::Builder &builder) : m_glorpDevice{device} {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}
GlorpModel::~GlorpModel() {
    vkDestroyBuffer(m_glorpDevice.device(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_glorpDevice.device(), m_vertexBufferMemory, nullptr);
    if(m_hasIndexBuffer) {
        vkDestroyBuffer(m_glorpDevice.device(), m_indexBuffer, nullptr);
        vkFreeMemory(m_glorpDevice.device(), m_indexBufferMemory, nullptr);
    }
}

void GlorpModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Create staging buffer
    m_glorpDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    
    void *data;
    vkMapMemory(m_glorpDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_glorpDevice.device(), stagingBufferMemory);

    m_glorpDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vertexBuffer,
        m_vertexBufferMemory
    );

    m_glorpDevice.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_glorpDevice.device(), stagingBuffer, nullptr);
    vkFreeMemory(m_glorpDevice.device(), stagingBufferMemory, nullptr);
}

void GlorpModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    m_indexCount = static_cast<uint32_t>(indices.size());
    m_hasIndexBuffer = m_indexCount > 0;

    if(!m_hasIndexBuffer) {
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // Create staging buffer
    m_glorpDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    
    void *data;
    vkMapMemory(m_glorpDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_glorpDevice.device(), stagingBufferMemory);

    m_glorpDevice.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_indexBuffer,
        m_indexBufferMemory
    );

    m_glorpDevice.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_glorpDevice.device(), stagingBuffer, nullptr);
    vkFreeMemory(m_glorpDevice.device(), stagingBufferMemory, nullptr);
}

void GlorpModel::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    
    if(m_hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void GlorpModel::draw(VkCommandBuffer commandBuffer) {
    if(m_hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }
}

std::vector<VkVertexInputBindingDescription> GlorpModel::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
} 
std::vector<VkVertexInputAttributeDescription> GlorpModel::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3,0,VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
}
std::unique_ptr<GlorpModel> GlorpModel::createModelFromFile(GlorpDevice &device, const std::string &filepath) {
    Builder builder{};
    builder.loadModel(filepath);
    return std::make_unique<GlorpModel>(device, builder);
}

void GlorpModel::Builder::loadModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices {};
    for(const auto &shape: shapes) {
        for(const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            if(index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }

            if(index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if(index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }
            if(uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }    
}
}