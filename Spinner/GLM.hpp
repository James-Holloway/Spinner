#ifndef SPINNER_GLM_HPP
#define SPINNER_GLM_HPP

#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

namespace Spinner
{
    constexpr static const glm::vec3 AxisForward = {0, 0, 1};
    constexpr static const glm::vec3 AxisBackward = {0, 0, -1};
    constexpr static const glm::vec3 AxisRight = {1, 0, 0};
    constexpr static const glm::vec3 AxisLeft = {-1, 0, 0};
    constexpr static const glm::vec3 AxisUp = {0, 1, 0};
    constexpr static const glm::vec3 AxisDown = {0, -1, 0};
}

#endif //SPINNER_GLM_HPP
