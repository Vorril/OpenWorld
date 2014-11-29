#include "vector2.h"


vector2::vector2()
{
	coords[0] = coords[1] = 0.0f;
}

vector2::vector2(float u, float v)
{
	coords[0] = u;
	coords[1] = v;
}

vector2::~vector2()
{
}
