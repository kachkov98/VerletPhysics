#pragma once
#include "window.h"
#include "vector.h"

class Camera
{
public:
	vector2d center;
	double scale;

	Camera () :
		scale (1.0)
	{ }

	void SetMatrix ()
	{
		double aspect_ratio = (double)width / (double)height;
		gluOrtho2D (center.x - scale * aspect_ratio,
					center.x + scale * aspect_ratio,
					center.y - scale,
					center.y + scale);
	}
};
