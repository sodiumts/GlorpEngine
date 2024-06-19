#pragma once

#include "glorp_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace Glorp {
struct TransformComponent {
    glm::vec3 translation {};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation {};

    glm::mat4 mat4();
    glm::mat3 normalMatrix();
};
class GlorpGameObject {
    public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, GlorpGameObject>;

    static GlorpGameObject createGameObject() {
        static id_t currentId = 0;
        return GlorpGameObject(currentId++);
    }

    GlorpGameObject(const GlorpGameObject&) = delete;
    GlorpGameObject &operator=(const GlorpGameObject &) = delete;

    GlorpGameObject(GlorpGameObject&&) = default;
    GlorpGameObject &operator=(GlorpGameObject &&) = default;


    id_t getId() {return id;}

    std::shared_ptr<GlorpModel> model;
    glm::vec3 color;
    TransformComponent transform {};

    private:
    GlorpGameObject(id_t objId) : id {objId} {}; 
    id_t id;

};
}