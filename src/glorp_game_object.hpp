#pragma once

#include "glorp_model.hpp"

#include <memory>

namespace Glorp {
struct Transform2dComponent {
    glm::vec2 translation {};
    glm::vec2 scale{1.f, 1.f};
    float rotation;

    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotationMat{{c, s}, {-s, c}};

        glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
        return rotationMat * scaleMat; 
    };

};
class GlorpGameObject {
    public:
    using id_t = unsigned int;

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
    Transform2dComponent transform2d {};

    private:
    GlorpGameObject(id_t objId) : id {objId} {}; 
    id_t id;

};
}