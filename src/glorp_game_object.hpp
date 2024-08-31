#pragma once

#include "glorp_model.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "glorp_texture.hpp"
#include <memory>
#include <unordered_map>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
struct TransformComponent {
    glm::vec3 translation {};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation {};

    glm::mat4 mat4();
    glm::mat3 normalMatrix();
};

struct PointLightComponent {
    float lightIntensity = 1.0f;
};

class GlorpGameObject {
    public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, GlorpGameObject>;

    static GlorpGameObject createGameObject() {
        static id_t currentId = 0;
        return GlorpGameObject(currentId++);
    }

    static GlorpGameObject createGameObjectFromFileGLTF(GlorpDevice &device, const std::string &filepath);

    GlorpGameObject(const GlorpGameObject&) = delete;
    GlorpGameObject &operator=(const GlorpGameObject &) = delete;

    GlorpGameObject(GlorpGameObject&&) = default;
    GlorpGameObject &operator=(GlorpGameObject &&) = default;


    id_t getId() {return id;}

    static GlorpGameObject makePointLight(float intensity = 10.f, float radius = 0.1, glm::vec3 color = glm::vec3(1.0f));

    glm::vec3 color;
    TransformComponent transform {};
    VkDescriptorSet descriptorSet;

    std::shared_ptr<GlorpTexture> texture;
    std::shared_ptr<GlorpModel> model;
    std::unique_ptr<PointLightComponent> pointLight = nullptr;
    private:
    GlorpGameObject(id_t objId) : id {objId} {}; 
    id_t id;

};
}