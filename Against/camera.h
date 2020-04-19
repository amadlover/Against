#pragma once

typedef struct
{
    float position[3];
    float rotation[3];
} camera;

void camera_set_position (camera* cam, float position[3]);
void camera_set_rotation (camera* cam, float rotation[3]);