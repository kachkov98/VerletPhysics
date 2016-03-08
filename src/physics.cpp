/**
@file
@brief implementation of physics framework
@author Sergei Kachkov
*/
#include <stdlib.h>
#include <stdarg.h>
#include "physics.h"
#include "log.h"

const double max_depth = 0.01;

struct Edge
{
	std::vector<Point>::iterator p1, p2;
};

struct CollisionInfo
{
	double depth;
	vector2d normal;
	Edge edge;
	std::vector<Point>::iterator point;
	size_t edge_body, point_body;
	vector2d static_point;
	bool is_point;
}info;

//----------implementation of point------------------------
Point::Point (vector2d position, double mass) :
	cur_pos (position),
	old_pos (position),
	m (mass)
{
	if (m < DBL_EPSILON)
		log (LOG_FAIL, "trying to create point with zero mass");
}

void Point::Update (double timestep, vector2d gravity)
{
	vector2d temp = cur_pos;
	cur_pos += (cur_pos - old_pos) + gravity * timestep * timestep * 0.5;
	old_pos = temp;
}
//----------end of implementation of point------------------

//----------impementation of pole---------------------------
Pole::Pole (size_t point1, size_t point2, double length) :
	p1 (point1),
	p2 (point2),
	len (length)
{
}
//----------end of implementation of pole-------------------

//----------implementation of static body-------------------
void StaticBody::AddStaticPoint (vector2d point)
{
	if (!points.size ())
		log (LOG_FAIL, "static body must have vertex(-es) before adding one more");
	for (size_t i = 0; i < points.size (); i++)
	{
		if (((points[(i + 1) % points.size ()] - points[i]) * (point - points[i])) < DBL_EPSILON)
		{
			points.insert (points.begin () + i + 1, point);
			if (point.x < bbox.lb.x)
				bbox.lb.x = point.x;
			if (point.x > bbox.rt.x)
				bbox.rt.x = point.x;
			if (point.y < bbox.lb.y)
				bbox.lb.y = point.y;
			if (point.y > bbox.rt.y)
				bbox.rt.y = point.y;
			break;
		}
	}
}

void StaticBody::ProjectToAxis (vector2d axis, double *min, double *max)
{
	*min = DBL_MAX;
	*max = -DBL_MAX;
	for (size_t i = 0; i < points.size (); i++)
	{
		double projection = axis ^ points[i];
		if (*max < projection)
			*max = projection;
		if (*min > projection)
			*min = projection;
	}
}

//----------end of implementation of static body------------

//----------implementation of dynamic body------------------
DynamicBody::DynamicBody (double k) :
	stiffness (k),
	mass (0.0)
{
}

void DynamicBody::RecalculateBBox ()
{
	bbox.lb = bbox.rt = points[0].cur_pos;
	for (size_t i = 1; i < points.size (); i++)
	{
		if (points[i].cur_pos.x < bbox.lb.x)
			bbox.lb.x = points[i].cur_pos.x;
		if (points[i].cur_pos.x > bbox.rt.x)
			bbox.rt.x = points[i].cur_pos.x;
		if (points[i].cur_pos.y < bbox.lb.y)
			bbox.lb.y = points[i].cur_pos.y;
		if (points[i].cur_pos.y > bbox.rt.y)
			bbox.rt.y = points[i].cur_pos.y;
	}
}

void DynamicBody::CalculateEdges ()
{
	edges.clear ();
	poles.clear ();
	for (size_t i = 0; i < points.size (); i++)
	{
		edges.push_back (Pole (i, (i + 1) % points.size (),
							   (points[i].cur_pos - points[(i + 1) % points.size ()].cur_pos).len ()));
	}
	for (size_t i = 0; i < points.size () - 3; i++)
		for (size_t j = 0; j < points.size () - i - 2; j++)
			poles.push_back (Pole (j, i + j + 2, (points[j].cur_pos - points[i + j + 2].cur_pos).len ()));
}

void DynamicBody::AddDynamicPoint (Point point)
{
	mass += point.m;
	for (size_t i = 0; i < points.size (); i++)
	{
		if (((points[(i + 1) % points.size ()].cur_pos - points[i].cur_pos) * (point.cur_pos - points[i].cur_pos)) < DBL_EPSILON)
		{
			points.insert (points.begin () + i + 1, point);
			break;
		}
	}
}

void DynamicBody::ProjectToAxis (vector2d axis, double *min, double *max)
{
	*min = DBL_MAX;
	*max = -DBL_MAX;
	for (size_t i = 0; i < points.size (); i++)
	{
		double projection = axis ^ (points[i].cur_pos);
		if (*max < projection)
			*max = projection;
		if (*min > projection)
			*min = projection;
	}
}

void DynamicBody::UpdatePoles ()
{
	//poles
	for (size_t i = 0; i < poles.size (); i++)
	{
		double inv_mass = 1 / (points[poles[i].p1].m + points[poles[i].p2].m);
		vector2d mass_center = (points[poles[i].p1].cur_pos * points[poles[i].p1].m +
								points[poles[i].p2].cur_pos * points[poles[i].p2].m) * inv_mass;
		vector2d norm = (points[poles[i].p2].cur_pos - points[poles[i].p1].cur_pos).norm ();
		//positions where points should be by spring non-stretched length
		vector2d goal1 = mass_center - norm * poles[i].len * points[poles[i].p2].m * inv_mass;
		vector2d goal2 = mass_center + norm * poles[i].len * points[poles[i].p1].m * inv_mass;
		points[poles[i].p1].cur_pos += (goal1 - points[poles[i].p1].cur_pos) * stiffness;
		points[poles[i].p2].cur_pos += (goal2 - points[poles[i].p2].cur_pos) * stiffness;
	}
	//edges
	for (size_t i = 0; i < edges.size (); i++)
	{
		double inv_mass = 1 / (points[edges[i].p1].m + points[edges[i].p2].m);
		vector2d mass_center = (points[edges[i].p1].cur_pos * points[edges[i].p1].m +
								points[edges[i].p2].cur_pos * points[edges[i].p2].m) * inv_mass;
		vector2d norm = (points[edges[i].p2].cur_pos - points[edges[i].p1].cur_pos).norm ();
		//positions where points should be by spring non-stretched length
		vector2d goal1 = mass_center - norm * edges[i].len * points[edges[i].p2].m * inv_mass;
		vector2d goal2 = mass_center + norm * edges[i].len * points[edges[i].p1].m * inv_mass;
		points[edges[i].p1].cur_pos += (goal1 - points[edges[i].p1].cur_pos) * stiffness;
		points[edges[i].p2].cur_pos += (goal2 - points[edges[i].p2].cur_pos) * stiffness;
	}
}
//----------end of implementation of dynamic body-----------

//----------implementation of physics-----------------------
Physics::Physics (double timestep, vector2d gravity, BoundingBox world_size) :
	t (timestep),
	a (gravity),
	world_box (world_size)
{
}

size_t Physics::AddStaticBody (size_t points_num, ...)
{
	if (points_num < 3)
		log (LOG_FAIL, "Static body must have at least 3 vertices");
	
	log (LOG_INFO, "adding static body with %u points", points_num);

	va_list valist;
	va_start (valist, points_num);
	StaticBodies.push_back (StaticBody ());
	vector2d v1, v2;
	v1 = va_arg (valist, vector2d);
	v2 = va_arg (valist, vector2d);
	StaticBodies.back().points.push_back (v1);
	StaticBodies.back().points.push_back (v2);
	
	//calculate bbox by two vertices
	if (v1.x < v2.x)
	{
		StaticBodies.back ().bbox.lb.x = v1.x;
		StaticBodies.back ().bbox.rt.x = v2.x;
	}
	else
	{
		StaticBodies.back ().bbox.lb.x = v2.x;
		StaticBodies.back ().bbox.rt.x = v1.x;
	}

	if (v1.y < v2.y)
	{
		StaticBodies.back ().bbox.lb.y = v1.y;
		StaticBodies.back ().bbox.rt.y = v2.y;
	}
	else
	{
		StaticBodies.back ().bbox.lb.y = v2.y;
		StaticBodies.back ().bbox.rt.y = v1.y;
	}

	//adding the other points
	for (size_t i = 2; i < points_num; i++)
		StaticBodies.back().AddStaticPoint( va_arg (valist, vector2d));

	va_end (valist);
	return StaticBodies.size () - 1;
}

size_t Physics::AddDynamicBody (double stiffness, size_t points_num, ...)
{
	if (points_num < 3)
		log (LOG_FAIL, "Dynamic body must have at least 3 verices");

	log (LOG_INFO, "adding dynamic body with %u points", points_num);

	va_list valist;
	va_start (valist, points_num);
	DynamicBodies.push_back (DynamicBody (stiffness));
	DynamicBodies.back ().points.push_back (va_arg (valist, Point));
	DynamicBodies.back ().mass += DynamicBodies.back ().points.back ().m;
	DynamicBodies.back ().points.push_back (va_arg (valist, Point));
	DynamicBodies.back ().mass += DynamicBodies.back ().points.back ().m;

	for (size_t i = 2; i < points_num; i++)
		DynamicBodies.back ().AddDynamicPoint (va_arg (valist, Point));

	va_end (valist);
	DynamicBodies.back ().RecalculateBBox ();
	DynamicBodies.back ().CalculateEdges ();
	return DynamicBodies.size () - 1;
}

bool Physics::isDynamicDynamic (size_t first, size_t second)
{
	size_t vertex_body = 0;
	info.depth = DBL_MAX;
	for (size_t i = 0; i < DynamicBodies[first].edges.size () + DynamicBodies[second].edges.size (); i++)
	{
		Edge edge;
		if (i < DynamicBodies[first].edges.size ())
		{
			edge.p1 = DynamicBodies[first].points.begin () + DynamicBodies[first].edges[i].p1;
			edge.p2 = DynamicBodies[first].points.begin () + DynamicBodies[first].edges[i].p2;
		}
		else
		{
			edge.p1 = DynamicBodies[second].points.begin () + 
				DynamicBodies[second].edges[i - DynamicBodies[first].edges.size ()].p1;
			edge.p2 = DynamicBodies[second].points.begin () +
				DynamicBodies[second].edges[i - DynamicBodies[first].edges.size ()].p2;
		}

		vector2d axis (edge.p2->cur_pos.y - edge.p1->cur_pos.y,
					   edge.p1->cur_pos.x - edge.p2->cur_pos.x);
		axis = axis.norm ();

		double first_min, first_max, second_min, second_max;
		DynamicBodies[first].ProjectToAxis (axis, &first_min, &first_max);
		DynamicBodies[second].ProjectToAxis (axis, &second_min, &second_max);

		double distance = (first_min < second_min) ? second_min - first_max : first_min - second_max;

		if (distance > -DBL_MIN)
			return false;
		else if (abs (distance) < info.depth)
		{
			info.depth = -distance;
			info.normal = axis;
			info.edge = edge;
			vertex_body = (i < DynamicBodies[first].edges.size ()) ? second : first;
		}
	}
	info.point = DynamicBodies[vertex_body].points.begin ();
	for (size_t i = 1; i < DynamicBodies[vertex_body].points.size (); i++)
		if ((info.normal ^ DynamicBodies[vertex_body].points[i].cur_pos) < (info.normal ^ info.point->cur_pos))
			info.point = DynamicBodies[vertex_body].points.begin () + i;
	info.point_body = vertex_body;
	info.edge_body = first + second - vertex_body;
	return true;
}

bool Physics::isDynamicStatic (size_t dynamic_body, size_t static_body)
{
	info.depth = DBL_MAX;
	info.is_point = false;
	//edges of dynamic body
	for (size_t i = 0; i < DynamicBodies[dynamic_body].edges.size (); i++)
	{
		Edge edge;
		edge.p1 = DynamicBodies[dynamic_body].points.begin () + DynamicBodies[dynamic_body].edges[i].p1;
		edge.p2 = DynamicBodies[dynamic_body].points.begin () + DynamicBodies[dynamic_body].edges[i].p2;

		vector2d axis (edge.p2->cur_pos.y - edge.p1->cur_pos.y,
					   edge.p1->cur_pos.x - edge.p2->cur_pos.x);
		axis = axis.norm ();

		double first_min, first_max, second_min, second_max;
		DynamicBodies[dynamic_body].ProjectToAxis (axis, &first_min, &first_max);
		StaticBodies[static_body].ProjectToAxis (axis, &second_min, &second_max);

		double distance = (first_min < second_min) ? second_min - first_max : first_min - second_max;

		if (distance > -DBL_MIN)
			return false;
		else if (abs (distance) < info.depth)
		{
			info.depth = -distance;
			info.normal = (second_max > first_max) ? axis : axis * (-1);;
			info.edge = edge;
		}
	}
	//edges of static body
	for (size_t i = 0; i < StaticBodies[static_body].points.size (); i++)
	{
		vector2d p1 = StaticBodies[static_body].points[i],
			p2 = StaticBodies[static_body].points[(i + 1) % StaticBodies[static_body].points.size ()];

		vector2d axis (p2.y - p1.y, p1.x - p2.x);
		axis = axis.norm ();

		double first_min, first_max, second_min, second_max;
		DynamicBodies[dynamic_body].ProjectToAxis (axis, &first_min, &first_max);
		StaticBodies[static_body].ProjectToAxis (axis, &second_min, &second_max);

		double distance = (first_min < second_min) ? second_min - first_max : first_min - second_max;

		if (distance > -DBL_MIN)
			return false;
		else if (abs (distance) < info.depth)
		{
			info.depth = -distance;
			info.normal = (first_max > second_max)? axis : axis * (-1);
			info.is_point = true;
		}
	}

	if (info.is_point)
	{
		info.point = DynamicBodies[dynamic_body].points.begin ();
		for (size_t i = 1; i < DynamicBodies[dynamic_body].points.size (); i++)
			if ((info.normal ^ DynamicBodies[dynamic_body].points[i].cur_pos) < (info.normal ^ info.point->cur_pos))
				info.point = DynamicBodies[dynamic_body].points.begin () + i;
	}
	else
	{
		info.static_point = StaticBodies[static_body].points[0];
		for (size_t i = 1; i < StaticBodies[static_body].points.size (); i++)
			if ((info.normal ^ StaticBodies[static_body].points[i]) < (info.normal ^ info.static_point))
				info.static_point = StaticBodies[static_body].points[i];
	}
	return true;
}

void Physics::Update (unsigned int iterations)
{
	//1st step: integration
	for (size_t i = 0; i < DynamicBodies.size (); i++)
	{
		for (size_t j = 0; j < DynamicBodies[i].points.size (); j++)
			DynamicBodies[i].points[j].Update (t, a);

		DynamicBodies[i].UpdatePoles ();

		DynamicBodies[i].RecalculateBBox ();
		if (!(DynamicBodies[i].bbox * world_box))
		{
			DynamicBodies.erase (DynamicBodies.begin () + i);
			log (LOG_INFO, "dynamic body destroyed");
		}
	}

	//2nd step: collision detection
	for (unsigned int cur_iteration = 0; cur_iteration < iterations; cur_iteration++)
	{
		for (size_t i = 0; i < DynamicBodies.size (); i++)
		{
			//dynamic - dynamic
			for (size_t j = i + 1; j < DynamicBodies.size (); j++)
				if (DynamicBodies[i].bbox * DynamicBodies[j].bbox)
					if (isDynamicDynamic (i, j))
					{
						// collision responce
						double sum_mass = DynamicBodies[info.edge_body].mass + DynamicBodies[info.point_body].mass;
						if (info.depth > max_depth)
							info.depth = max_depth;
						info.point->cur_pos += info.normal * info.depth * (DynamicBodies[info.edge_body].mass / sum_mass);

						double t = (info.point->cur_pos - info.edge.p1->cur_pos).len () * info.edge.p1->m /
							((info.point->cur_pos - info.edge.p1->cur_pos).len () * info.edge.p1->m +
							 (info.point->cur_pos - info.edge.p2->cur_pos).len () * info.edge.p2->m);
						double lambda = 1.0 / (t * t + (1 - t) * (1 - t));
						info.edge.p1->cur_pos -= info.normal * info.depth * (1 - t) * 0.5 * lambda;
						info.edge.p2->cur_pos -= info.normal * info.depth * t * 0.5 * lambda;

						DynamicBodies[i].RecalculateBBox ();
						DynamicBodies[j].RecalculateBBox ();
					}
			//dynamic - static
			for (size_t j = 0; j < StaticBodies.size (); j++)
				if (DynamicBodies[i].bbox * StaticBodies[j].bbox)
					if (isDynamicStatic (i, j))
					{
						// collision responce
						if (info.depth > max_depth)
							info.depth = max_depth;
						if (info.is_point)
							info.point->cur_pos += info.normal * info.depth;
						else
						{
							double t = (info.static_point - info.edge.p1->cur_pos).len() * info.edge.p1->m /
								((info.edge.p1->cur_pos - info.static_point).len () * info.edge.p1->m +
								 (info.edge.p2->cur_pos - info.static_point).len () * info.edge.p2->m);
							double lambda = 1.0 / (t * t + (1 - t) * (1 - t));
							info.edge.p1->cur_pos -= info.normal * info.depth * (1-t) * lambda;
							info.edge.p2->cur_pos -= info.normal * info.depth * t * lambda;
						}
						DynamicBodies[i].RecalculateBBox ();
					}
		}
	}
}
//----------end of implementation of physics----------------