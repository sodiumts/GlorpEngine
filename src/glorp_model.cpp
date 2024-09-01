#include "glorp_model.hpp"

#include "first_app.hpp"
#include "glorp_utils.hpp"

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <memory>

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
GlorpModel::~GlorpModel() {}

void computeTangentsAndBitangents(std::vector<GlorpModel::Vertex>& vertices, std::vector<uint32_t>& indices) {
    std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> bitangents(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t idx0 = indices[i];
        uint32_t idx1 = indices[i + 1];
        uint32_t idx2 = indices[i + 2];

        const auto& v0 = vertices[idx0];
        const auto& v1 = vertices[idx1];
        const auto& v2 = vertices[idx2];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.uv - v0.uv;
        glm::vec2 deltaUV2 = v2.uv - v0.uv;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        tangents[idx0] += tangent;
        tangents[idx1] += tangent;
        tangents[idx2] += tangent;

        bitangents[idx0] += bitangent;
        bitangents[idx1] += bitangent;
        bitangents[idx2] += bitangent;
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].tangent = glm::normalize(tangents[i]);
        vertices[i].bitangent = glm::normalize(bitangents[i]);
    }
}

void GlorpModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

    uint32_t vertexSize = sizeof(vertices[0]);

    GlorpBuffer stagingBuffer {
        m_glorpDevice,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    m_vertexBuffer = std::make_unique<GlorpBuffer>(
        m_glorpDevice,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_glorpDevice.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
}

void GlorpModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    m_indexCount = static_cast<uint32_t>(indices.size());
    m_hasIndexBuffer = m_indexCount > 0;

    if(!m_hasIndexBuffer) {
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

    uint32_t indexSize = sizeof(indices[0]);

    GlorpBuffer stagingBuffer {
        m_glorpDevice,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *) indices.data());

    m_indexBuffer = std::make_unique<GlorpBuffer> (
        m_glorpDevice,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );


    m_glorpDevice.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
}

void GlorpModel::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if(m_hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
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
    attributeDescriptions.push_back({4,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)});
    attributeDescriptions.push_back({5,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, bitangent)});

    return attributeDescriptions;
}

std::unique_ptr<GlorpModel> GlorpModel::createModelFromGLTF(GlorpDevice &device, tinygltf::Model &model) {
    Builder builder{};
    builder.loadModelFromGLTF(model);

    return std::make_unique<GlorpModel>(device, builder);
}

void GlorpModel::Builder::loadModelFromGLTF(tinygltf::Model &model) {
    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            // Load vertex positions
            const auto& positionAccessor = model.accessors.at(primitive.attributes.at("POSITION"));
            const auto& positionBufferView = model.bufferViews.at(positionAccessor.bufferView);
            const auto& positionBuffer = model.buffers.at(positionBufferView.buffer);
            const float* positions = reinterpret_cast<const float*>(&positionBuffer.data[positionBufferView.byteOffset + positionAccessor.byteOffset]);

            // Load normals
            std::vector<glm::vec3> normals;
            if (primitive.attributes.count("NORMAL")) {
                const auto& normalAccessor = model.accessors.at(primitive.attributes.at("NORMAL"));
                const auto& normalBufferView = model.bufferViews.at(normalAccessor.bufferView);
                const auto& normalBuffer = model.buffers.at(normalBufferView.buffer);
                const float* normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);
                normals.resize(normalAccessor.count);
                for (size_t i = 0; i < normalAccessor.count; ++i) {
                    normals[i] = { normalData[i * 3 + 0], normalData[i * 3 + 1], normalData[i * 3 + 2] };
                }
            }

            // Load UVs
            std::vector<glm::vec2> uvs;
            if (primitive.attributes.count("TEXCOORD_0")) {
                const auto& uvAccessor = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
                const auto& uvBufferView = model.bufferViews.at(uvAccessor.bufferView);
                const auto& uvBuffer = model.buffers.at(uvBufferView.buffer);
                const float* uvData = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
                uvs.resize(uvAccessor.count);
                for (size_t i = 0; i < uvAccessor.count; ++i) {
                    uvs[i] = { uvData[i * 2 + 0], uvData[i * 2 + 1] };
                }
            }

            // Load Colors
            std::vector<glm::vec3> colors;
            if (primitive.attributes.count("COLOR_0")) {
                const auto& colorAccessor = model.accessors.at(primitive.attributes.at("COLOR_0"));
                const auto& colorBufferView = model.bufferViews.at(colorAccessor.bufferView);
                const auto& colorBuffer = model.buffers.at(colorBufferView.buffer);
                const float* colorData = reinterpret_cast<const float*>(&colorBuffer.data[colorBufferView.byteOffset + colorAccessor.byteOffset]);
                colors.resize(colorAccessor.count);
                for (size_t i = 0; i < colorAccessor.count; ++i) {
                    colors[i] = { colorData[i * 3 + 0], colorData[i * 3 + 1], colorData[i * 3 + 2] };
                }
            }

            // Process indices
            if (primitive.indices > -1) {
                const auto& indexAccessor = model.accessors.at(primitive.indices);
                const auto& indexBufferView = model.bufferViews.at(indexAccessor.bufferView);
                const auto& indexBuffer = model.buffers.at(indexBufferView.buffer);
                const uint8_t* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

                switch (indexAccessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                        const uint8_t* indicesUByte = reinterpret_cast<const uint8_t*>(indexData);
                        for (size_t i = 0; i < indexAccessor.count; ++i) {
                            Vertex vertex{};
                            vertex.position = {
                                positions[indicesUByte[i] * 3 + 0],
                                positions[indicesUByte[i] * 3 + 2],
                                -positions[indicesUByte[i] * 3 + 1]
                            };
                            vertex.normal = vertex.normal = !normals.empty() ? glm::vec3(
                                    normals[indicesUByte[i]].x,
                                    normals[indicesUByte[i]].z,
                                    -normals[indicesUByte[i]].y
                                ) : glm::vec3(0.0f, 0.0f, 1.0f);

                            vertex.uv = !uvs.empty() ? uvs[indicesUByte[i]] : glm::vec2(0.0f, 0.0f);
                            vertex.color = !colors.empty() ? colors[indicesUByte[i]] : glm::vec3(1.0f, 1.0f, 1.0f);

                            if (uniqueVertices.count(vertex) == 0) {
                                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                                vertices.push_back(vertex);
                            }
                            indices.push_back(uniqueVertices[vertex]);
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        const uint16_t* indicesUShort = reinterpret_cast<const uint16_t*>(indexData);
                        for (size_t i = 0; i < indexAccessor.count; ++i) {
                            Vertex vertex{};
                            vertex.position = {
                                positions[indicesUShort[i] * 3 + 0],
                                positions[indicesUShort[i] * 3 + 2],
                                -positions[indicesUShort[i] * 3 + 1]
                            };
                            vertex.normal = vertex.normal = !normals.empty() ? glm::vec3(
                                    normals[indicesUShort[i]].x,
                                    normals[indicesUShort[i]].z,
                                    -normals[indicesUShort[i]].y
                                ) : glm::vec3(0.0f, 0.0f, 1.0f);

                            vertex.uv = !uvs.empty() ? uvs[indicesUShort[i]] : glm::vec2(0.0f, 0.0f);
                            vertex.color = !colors.empty() ? colors[indicesUShort[i]] : glm::vec3(1.0f, 1.0f, 1.0f);

                            if (uniqueVertices.count(vertex) == 0) {
                                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                                vertices.push_back(vertex);
                            }
                            indices.push_back(uniqueVertices[vertex]);
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        const uint32_t* indicesUInt = reinterpret_cast<const uint32_t*>(indexData);
                        for (size_t i = 0; i < indexAccessor.count; ++i) {
                            Vertex vertex{};
                            vertex.position = {
                                positions[indicesUInt[i] * 3 + 0],
                                positions[indicesUInt[i] * 3 + 2],
                                -positions[indicesUInt[i] * 3 + 1]
                            };
                            vertex.normal = vertex.normal = !normals.empty() ? glm::vec3(
                                normals[indicesUInt[i]].x,
                                normals[indicesUInt[i]].z,
                                -normals[indicesUInt[i]].y
                            ) : glm::vec3(0.0f, 0.0f, 1.0f);
                            vertex.uv = !uvs.empty() ? uvs[indicesUInt[i]] : glm::vec2(0.0f, 0.0f);
                            vertex.color = !colors.empty() ? colors[indicesUInt[i]] : glm::vec3(1.0f, 1.0f, 1.0f);

                            if (uniqueVertices.count(vertex) == 0) {
                                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                                vertices.push_back(vertex);
                            }
                            indices.push_back(uniqueVertices[vertex]);
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("Unsupported index type");
                }
            }
        }
    }
    computeTangentsAndBitangents(vertices, indices);
}
}
