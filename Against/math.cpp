#include "math.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

void create_matrix_from_translation (float* translation, float* out_matrix)
{
    memcpy (out_matrix, glm::value_ptr (glm::translate (glm::mat4 (1.0f), glm::make_vec3 (translation))), sizeof (float) * 16);
}

void create_matrix_from_rotation (float* rotation, float* out_matrix)
{
    memcpy (out_matrix, glm::value_ptr (glm::toMat4 (glm::make_quat (rotation))), sizeof (float) * 16);
}

void create_matrix_from_scale (float* scale, float* out_matrix)
{
    memcpy (out_matrix, glm::value_ptr (glm::scale (glm::mat4 (1.0f), glm::make_vec3 (scale))), sizeof (float) * 16);
}

void translate_matrix (float* matrix, float* translation, float* out_matrix)
{
    memcpy (out_matrix, glm::value_ptr (glm::translate (glm::make_mat4 (matrix), glm::make_vec3 (translation))), sizeof (float) * 16);
}

void rotate_matrix (float* matrix, float* rotation, float* out_matrix)
{
}

void scale_matrix (float* matrix, float* scale, float* out_matrix)
{
    memcpy (out_matrix, glm::value_ptr (glm::scale (glm::make_mat4 (matrix), glm::make_vec3 (scale))), sizeof (float) * 16);
}

void create_perspective_projection_matrix (float fov, float aspect_ratio, float, float, float* out_matrix)
{
}
