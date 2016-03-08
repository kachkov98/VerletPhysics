/**
@file
@brief implementation of bounding box
*/
#pragma once
#include "vector.h"

/**
@class
@brief implements collision detection of bounding boxes
*/
struct BoundingBox
{
	vector2d lb, rt;
	/*
	@brief creates bounding box
	@note bounding box by default will not intersect to other boxes
	*/
	BoundingBox () :
		lb (vector2d (DBL_MAX, DBL_MAX)),
		rt (vector2d (-DBL_MAX, -DBL_MAX))
	{
	}

	/**
	@brief creates bounding box
	@param left_bottom vector of left bottom corner
	@param right_top vector of right top corner
	@warning constructor doesn't check the order of corners in release
	*/
	BoundingBox (vector2d left_bottom, vector2d right_top) :
		lb (left_bottom),
		rt (right_top)
	{
		#ifdef DEBUG
		if (lb.x > rt.x)
		{
			double t = lb.x;
			lb.x = rt.x;
			rt.x = t;
		}
		if (lb.y > rt.y)
		{
			double t = lb.y;
			lb.y = rt.y;
			rt.y = t;
		}
		#endif // DEBUG
	}

	/**
	@brief check intersection of two boxes
	@return true, if bounding boxes intersects
	*/
	bool operator* (BoundingBox b)
	{
		if ((rt.x < b.lb.x) || (lb.x > b.rt.x))
			return false;
		if ((rt.y < b.lb.y) || (lb.y > b.rt.y))
			return false;
		return true;
	}
};