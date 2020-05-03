#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    void math_create_identity_matrix (float* out_matrix);

    void math_create_matrix_from_translation (float* translation, float* out_matrix);
    void math_create_matrix_from_rotation (float* rotation, float* out_matrix);
    void math_create_matrix_from_scale (float* scale, float* out_matrix);

    void math_translate_matrix (float* matrix, float* translation, float* out_matrix);
    void math_rotate_matrix (float* matrix, float* rotation, float* out_matrix);
    void math_scale_matrix (float* matrix, float* scale, float* out_matrix);

    void math_create_perspective_projection_matrix (float fov, float aspect_ratio, float near, float far, float* out_matrix);
    void math_create_orthographic_projection_matrix (float top, float bottom, float left, float right, float near, float far, float* out_matrix);
#ifdef __cplusplus
};
#endif
