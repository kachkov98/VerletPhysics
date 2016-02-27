/**
@file
@brief physics framework
@author Sergei Kachkov
*/
#pragma once
#include <vector>
#include "vector.h"
#include "boundingbox.h"

/**
@class
@brief mass point
*/
class Point
{
private:
	vector2d old_pos;
public:
	double m;
	vector2d cur_pos;

	/**
	@brief constructor of mass point
	@param position current position of mass point
	@param mass positive mass
	@note velocity is zero by default; velocity defined implicitly via old_pos and timestep
	*/
	Point (vector2d position, double mass);
	/**
	@brief Verlet integration of point
	@param timestep timestep of simulation
	@param gravity gravity for this point
	@note variable gravity can be useful for central-force simulation
	*/
	void Update (double timestep, vector2d gravity);
};

/**
@class
@brief spring object
@warning not physical-accurately; approximation of damped oscillations
*/
class Pole
{
private:
	double len;
public:
	std::vector<Point>::iterator p1, p2;
	/**
	@brief constructor of point
	@param point1, point2 iterators to points that should be connected
	@param length length of non-stretched pole
	*/
	Pole (std::vector<Point>::iterator point1, std::vector<Point>::iterator point2, double length);
	/**
	@brief constructor of point
	@param point1, point2 iterators to points that should be connected
	@note non-stretched length of pole calculates automatically
	*/
	Pole (std::vector<Point>::iterator point1, std::vector<Point>::iterator point2);
	/**
	@brief updates position of points that connected to pole
	@param stiffness stiffness of spring
	*/
	void Update (double stiffness);
};

/**
@class
@brief Static Body shape
*/
class StaticBody
{
public:
	std::vector<vector2d> points;
	BoundingBox bbox;
	/**
	@brief adds vertex to figure; checks on convexity and orientation
	@param point coordinates of vertex
	@warning body must have at least 1 vertex before calling this method
	*/
	void AddStaticPoint (vector2d point);
	/**
	@brief project body on axis
	@param axis normalized projection axis
	@param min, max pointers to variables of projection coordinates
	*/
	void ProjectToAxis (vector2d axis, double *min, double *max);
};

/**
@class
@brief Dynamic Body shape
*/
class DynamicBody
{
public:
	double stiffness;
	double mass;
	std::vector<Point> points;
	std::vector<Pole> poles;
	std::vector<Pole> edges;
	BoundingBox bbox;
	/**
	@brief constructor of Dynamic Body
	@param k stiffness of body
	*/
	DynamicBody (double k);
	/**
	@brief calculate new bounding box
	*/
	void RecalculateBBox ();
	/**
	@brief adds dynamic point to the body
	@note this method hase very limited usecase: adding points during generation of body
	@warning after adding point, call CalculateEdges() manually to update poles
	@param point mass point
	*/
	void AddDynamicPoint (Point point);
	/**
	@brief updates poles and edges
	@warning information about non-stretched lengths removes; method use current lengths between points
	*/
	void CalculateEdges ();
	/**
	@brief project body on axis
	@param axis normalized projection axis
	@param min, max pointers to variables of projection coordinates
	*/
	void ProjectToAxis (vector2d axis, double *min, double *max);
};

struct CollisionInfo
{
	double depth;
	vector2d normal;
	std::vector<Pole>::iterator edge;
	std::vector<Point>::iterator point;
	size_t edge_body, point_body;
	vector2d static_point;
	bool is_point;
};

/**
@class
@brief physics engine
*/
class Physics
{
private:
	double t;
	vector2d a;
	BoundingBox world_box;
	CollisionInfo info;
	/**
	@brief collision detection between dynamic bodies
	@param first, second indexes of bodies
	@param info collision information if bodies intersect
	@return true, if shapes intersect
	*/
	bool isDynamicDynamic (size_t first, size_t second, CollisionInfo *info);
	/**
	@brief collision detection between static and dynamic bodies
	@param dynamic_body, static_body indexes of bodies
	@param info collision information if bodies intersect
	@return true, if shapes intersect
	*/
	bool isDynamicStatic (size_t dynamic_body, size_t static_body, CollisionInfo *info);
public:
	std::vector<StaticBody> StaticBodies;
	std::vector<DynamicBody> DynamicBodies;

	/**
	@brief constructor of physics engine
	@param timestep step of simulation (in verlet integration must be constant)
	@param gravity vector of gravity that applies to all bodies
	@param world_size bounding box of world; when body leaves this box, it automatically deletes
	*/
	Physics (double timestep, vector2d gravity, BoundingBox world_size);
	/**
	@brief adds static body to world
	@param points_num number of points; must be at least 3
	@return index of created body in StaticBodies array
	*/
	size_t AddStaticBody (size_t points_num, ...);
	/**
	@brief adds dynamic body to world
	@param stiffness stiffness of all poles in body
	@param points_num number of points; must be at least 3
	@return index of created body in DynamicBodies array
	@warning it doesn't guarantee that this index will valid after Update method;
	you should add points to body by this index directly after this method
	*/
	size_t AddDynamicBody (double stiffness, size_t points_num, ...);
	/**
	@brief updates world; main method of simulation
	@param iterations number of resolve iterations
	*/
	void Update (unsigned int iterations);
};
