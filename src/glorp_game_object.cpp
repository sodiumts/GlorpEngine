#include "glorp_game_object.hpp"
#include <memory>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include <iostream>
#include <chrono>

namespace Glorp {
glm::mat4 TransformComponent::mat4() {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f}
    };
}
glm::mat3 TransformComponent::normalMatrix() {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 invScale = 1.0f / scale;

    return glm::mat3{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
        }
    };
}

GlorpGameObject GlorpGameObject::createGameObjectFromAscii(GlorpDevice &device, const std::string &filepath) {
    std::string fullPath = RESOURCE_LOCATIONS + filepath;
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    auto start = std::chrono::high_resolution_clock::now();
    bool res = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, fullPath);
    if(!warn.empty()) {
        std::cout << "Warning from loading gltf file: " << warn << std::endl;
    }
    if(!err.empty()) {
        std::cout << "Error loading gltf file: " << err << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to load gltf file " << fullPath << ": " << duration.count() << " seconds" << std::endl;

    GlorpGameObject gameObject = GlorpGameObject::createGameObject();
    assembleGameObject(device, gameObject, gltfModel);

    return gameObject;
}

// I know this is ugly, but I'm not sure how to handle this better.
GlorpGameObject GlorpGameObject::createGameObjectFromBin(GlorpDevice &device, const std::string &filepath) {
    std::string fullPath = RESOURCE_LOCATIONS + filepath;
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    auto start = std::chrono::high_resolution_clock::now();
    bool res = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, fullPath);
    if(!warn.empty()) {
        std::cout << "Warning from loading gltf file: " << warn << std::endl;
    }
    if(!err.empty()) {
        std::cout << "Error loading gltf file: " << err << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to load gltf file " << fullPath << ": " << duration.count() << " seconds" << std::endl;

    GlorpGameObject gameObject = GlorpGameObject::createGameObject();
    assembleGameObject(device, gameObject, gltfModel);

    return gameObject;
}

void GlorpGameObject::assembleGameObject(GlorpDevice &device, GlorpGameObject &gameObject, tinygltf::Model &gltfModel) {
    gameObject.model = GlorpModel::createModelFromGLTF(device, gltfModel);
    //TODO:: Add more error checking for missing emmision for example.
    auto materialComponent = std::make_unique<MaterialComponent>();
    for (const auto& material : gltfModel.materials) {
        // Handle baseColorTexture
        if (material.values.find("baseColorTexture") != material.values.end()) {
            std::cout << "Found baseColorTexture" << std::endl;
            const auto& baseColorTexture = material.values.at("baseColorTexture");
            if(baseColorTexture.TextureIndex() >= 0) {
                const tinygltf::Texture& texture = gltfModel.textures[baseColorTexture.TextureIndex()];
                const tinygltf::Image& image = gltfModel.images[texture.source];
                materialComponent->albedoTexture = std::make_shared<GlorpTexture>(device, image);
            }
        }

        // Handle metallic roughness 
        for (const auto&material : gltfModel.materials) {
            if(material.values.find("metallicRoughnessTexture") != material.values.end()) {
                std::cout << "Found metallic roughness texture" << std::endl;
                const auto& metallicRoughnessTexture = material.values.at("metallicRoughnessTexture");
                if(metallicRoughnessTexture.TextureIndex() >= 0) {
                    const tinygltf::Texture& texture = gltfModel.textures[metallicRoughnessTexture.TextureIndex()];
                    const tinygltf::Image& image = gltfModel.images[texture.source];
                    materialComponent->metallicRoughnessTexture = std::make_shared<GlorpTexture>(device, image);
            }   
            }
        }


        // Handle occlusionTexture
        {
            const tinygltf::Texture& texture = gltfModel.textures[material.occlusionTexture.index];
            const tinygltf::Image& image = gltfModel.images[texture.source];
            materialComponent->aoTexture = std::make_shared<GlorpTexture>(device, image);
        }

        // Handle emissiveTexture
        {
            const tinygltf::Texture& texture = gltfModel.textures[material.emissiveTexture.index];
            const tinygltf::Image& image = gltfModel.images[texture.source];
            materialComponent->emissiveTexture = std::make_shared<GlorpTexture>(device, image);
        }

        // Handle normalTexture
        {
            const tinygltf::Texture& texture = gltfModel.textures[material.normalTexture.index];
            const tinygltf::Image& image = gltfModel.images[texture.source];
            materialComponent->normalTexture = std::make_shared<GlorpTexture>(device, image);
        }
    }
    gameObject.material = std::move(materialComponent);
}

GlorpGameObject GlorpGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
    GlorpGameObject gameObject = GlorpGameObject::createGameObject();
    gameObject.color = color;
    gameObject.transform.scale.x = radius;
    gameObject.pointLight = std::make_unique<PointLightComponent>();
    gameObject.pointLight->lightIntensity = intensity;
    return gameObject;
}

}
