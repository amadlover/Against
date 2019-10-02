#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	void CreateOrthographicMatrixGLM (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result);
	void CreateTransformationMatrix (float* Position, float* Rotation, float* Scale, float* Result);

	void CreateViewProjectionMatrix (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Position, float* Rotation, float* Scale, float* Result);

#ifdef __cplusplus
}
#endif
