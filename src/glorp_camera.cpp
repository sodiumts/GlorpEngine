#include "glorp_camera.hpp"

#include <cassert>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>

namespace Glorp {
void GlorpCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) {
    m_projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void GlorpCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    m_projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

void GlorpCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    const glm::quat orientation = glm::quatLookAt(glm::normalize(direction), glm::normalize(up));

    m_viewMatrix = glm::translate(glm::mat4(1.0f), -position) * glm::mat4_cast(glm::conjugate(orientation));

    m_inverseViewMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orientation);
}

void GlorpCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    setViewDirection(position, target - position, up);
}

void GlorpCamera::setViewYXZ(glm::vec3 position, glm::quat rotation) {
    m_viewMatrix = glm::mat4_cast(glm::conjugate(rotation)) * glm::translate(glm::mat4(1.0f), -position);
    m_inverseViewMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
}
}
