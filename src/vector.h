/**
@file
@brief implementation of 2d vector
@author Sergei Kachkov
*/
#pragma once
#include <math.h>
#include <float.h>
#include "log.h"

/**
@class
@brief 2d vector
*/
class vector2d
{
public:
	double x, y;

	/**
	@brief creates vector
	@param v_x, v_y coordinates (0 by default)
	*/
	vector2d (double v_x = 0, double v_y = 0) :
		x (v_x),
		y (v_y)
	{ }

	/**
	@return length of vector
	*/
	double len ()
	{
		return sqrt (x*x + y*y);
	}

	/**
	@return squared length of vector
	@note much faster than square of len () method
	*/
	double sqr_len ()
	{
		return x*x + y*y;
	}

	/*
	@brief normalizes vector
	@warning method check on 0-vectors only in debug mode
	*/
	vector2d norm ()
	{
		#ifdef _DEBUG
		if (x*x + y*y < DBL_EPSILON)
			log (LOG_FAIL, "normalize of 0-vector");
		#endif
		double scale = 1 / sqrt (x*x + y*y);
		return vector2d (x * scale, y * scale);
	}

	vector2d operator+ (vector2d b)
	{
		return vector2d (x + b.x, y + b.y);
	}

	vector2d operator- (vector2d b)
	{
		return vector2d (x - b.x, y - b.y);
	}

	vector2d operator+= (vector2d b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	vector2d operator-= (vector2d b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	/**
	@brief calculates pseudoscalar product of vectors
	@warning not scalar! pseudoscalar product is oriented area of parallelogram
	*/
	double operator* (vector2d b)
	{
		return x * b.y - y * b.x;
	}

	vector2d operator* (double b)
	{
		return vector2d (x * b, y * b);
	}

	/**
	@brief calculates dot product
	*/
	double operator^ (vector2d b)
	{
		return x * b.x + y * b.y;
	}

	vector2d operator/ (double b)
	{
		return vector2d (x / b, y / b);
	}

	/**
	@brief print vector coordinates in log; format: vector2d [name] (x y)
	@param name name of vector that will print in log
	*/
	void print (const char *name)
	{
		log (LOG_INFO, "vector2d %s (%0.5lf %0.5lf)", name, x, y);
	}
};
