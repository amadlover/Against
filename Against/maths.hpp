#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	void CreateOrthographicMatrixGLM (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result);
	void CreatePerspectiveProjectionMatrixGLM (float FOV, float AspectRatio, float Near, float Far, float* Result);
	void CreateLookatMatrixGLM (float* Eye, float* Center, float* Up, float* Result);
	void CreateTransformationMatrixGLM (float* Position, float* Rotation, float* Scale, float* Result);

	void CreateModelViewProjectinMatrix (float* Projection, float* View, float* Model, float* Result);

	void MultiplyMatricesGLM (float* Mat1, float* Mat2, float* Result);

	/*void CreateOrthographicMatrix (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result);
	void CreatePerspectiveMatrix (float AspectRatio, float FOV, float Near, float Far, float* Result);*/
#ifdef __cplusplus
}
#endif
