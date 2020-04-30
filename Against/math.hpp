#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    void create_matrix_from_translation (float* translation, float* out_matrix);
    void create_matrix_from_rotation (float* rotation, float* out_matrix);
    void create_matrix_from_scale (float* scale, float* out_matrix);

    void translate_matrix (float* matrix, float* translation, float* out_matrix);
    void rotate_matrix (float* matrix, float* rotation, float* out_matrix);
    void scale_matrix (float* matrix, float* scale, float* out_matrix);

    void create_perspective_projection_matrix (float fov, float aspect_ratio, float near, float far, float* out_matrix);
#ifdef __cplusplus
};
#endif
