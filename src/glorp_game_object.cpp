#include "glorp_game_object.hpp"
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <memory>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include <iostream>
#include <chrono>

namespace Glorp {
glm::mat4 TransformComponent::mat4() {
    glm::mat4 model = glm::mat4(1.f);
    model = glm::scale(model, scale);
    model *= glm::mat4_cast(rotation);
    model = glm::translate(model, translation);

    return model;
}

glm::mat3 TransformComponent::normalMatrix() {
    return glm::mat3(glm::transpose(glm::inverse(mat4())));
}

GlorpGameObject GlorpGameObject::createGameObjectFromAscii(GlorpDevice &device, const std::string &filepath) {
    tinygltf::Model gameObjectModel;
    loadAsciiGLTF(gameObjectModel, filepath);

    GlorpGameObject gameObject = GlorpGameObject::createGameObject();
    assembleGameObject(device, gameObject, gameObjectModel);

    return gameObject;
}

void GlorpGameObject::loadBinaryGLTF(tinygltf::Model &model, const std::string &filepath) {
    std::string fullPath = RESOURCE_LOCATIONS + filepath;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    auto start = std::chrono::high_resolution_clock::now();
    bool res = loader.LoadBinaryFromFile(&model, &err, &warn, fullPath);
    if(!warn.empty()) {
        std::cout << "Warning from loading gltf file: " << warn << std::endl;
    }
    if(!err.empty()) {
        std::cout << "Error loading gltf file: " << err << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to load gltf file " << fullPath << ": " << duration.count() << " seconds" << std::endl;
}

void GlorpGameObject::loadAsciiGLTF(tinygltf::Model &model, const std::string &filepath) {
    std::string fullPath = RESOURCE_LOCATIONS + filepath;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    auto start = std::chrono::high_resolution_clock::now();
    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, fullPath);
    if(!warn.empty()) {
        std::cout << "Warning from loading gltf file: " << warn << std::endl;
    }
    if(!err.empty()) {
        std::cout << "Error loading gltf file: " << err << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken to load gltf file " << fullPath << ": " << duration.count() << " seconds" << std::endl;
}

GlorpGameObject GlorpGameObject::createGameObjectFromBin(GlorpDevice &device, const std::string &filepath) {
    tinygltf::Model gameObjectModel;
    loadBinaryGLTF(gameObjectModel, filepath);
    
    GlorpGameObject gameObject = GlorpGameObject::createGameObject();
    assembleGameObject(device, gameObject, gameObjectModel);

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
            if (material.occlusionTexture.index != -1) {
                const tinygltf::Texture& texture = gltfModel.textures[material.occlusionTexture.index];
                const tinygltf::Image& image = gltfModel.images[texture.source];
                materialComponent->aoTexture = std::make_shared<GlorpTexture>(device, image);
            }
        }

        // Handle emissiveTexture
        {
            if(material.emissiveTexture.index != -1) {
                const tinygltf::Texture& texture = gltfModel.textures[material.emissiveTexture.index];
                const tinygltf::Image& image = gltfModel.images[texture.source];
                materialComponent->emissiveTexture = std::make_shared<GlorpTexture>(device, image);
            }
        }

        // Handle normalTexture
        {
            if(material.normalTexture.index != -1) {
                const tinygltf::Texture& texture = gltfModel.textures[material.normalTexture.index];
                const tinygltf::Image& image = gltfModel.images[texture.source];
                materialComponent->normalTexture = std::make_shared<GlorpTexture>(device, image);
            }
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
