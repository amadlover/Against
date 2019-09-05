#include "MathsCPP.hpp"

void TestMathsCPP ()
{
	Matrix4x4 Result = MatrixGetIdentity ();
	CreatePerspectiveProjectionMatrix (90, 1, 0.1f, 100, &Result);
}