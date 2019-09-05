#pragma once

#include "Maths.h"

#ifdef __cplusplus
extern "C" 
{
#endif
	void CreatePerspectiveProjectionMatrix (float FOV, float AspectRatio, float Near, float Far, Matrix4x4* Result);
#ifdef __cplusplus
}
#endif