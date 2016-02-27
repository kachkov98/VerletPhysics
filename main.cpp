/**
@file
@brief main program that uses physics and application framework
@author Sergei Kachkov
*/

#include "physics.h"
#include "window.h"

double timestep = 0.002;
Physics world (timestep, vector2d (0, -9.8), BoundingBox (vector2d(-100, -100), vector2d(100, 100)));

/**
@brief Sets OpenGL parameters and generates physics objects
*/
void init ()
{
	// OpenGL parameters
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glEnable (GL_POINT_SMOOTH);
	glPointSize (2.0);

	//generate world
	world.AddStaticBody (4, vector2d (-5.0, -1.2), vector2d (-5.0, -1.0), vector2d (5.0, -1.0), vector2d (5.0, -1.2));
	world.AddStaticBody (4, vector2d (-5.0, -1.2), vector2d (-5.2, -1.2), vector2d (-5.2, 2.0), vector2d (-5.0, 2.0));
	world.AddStaticBody (4, vector2d (5.0, -1.2), vector2d (5.2, -1.2), vector2d (5.2, 2.0), vector2d (5.0, 2.0));

	for (int i = -2; i <= 2; i+=2)
		world.AddStaticBody (5, vector2d (0.0 + i, 1.0),  vector2d (0.5 + i, 0.5),
							    vector2d (-0.5 + i, 0.5), vector2d (0.5 + i, 0.0), vector2d (-0.5 + i, 0.0));

	for (double i = -1.5; i <= 1.5; i+=1.5)
		world.AddDynamicBody (0.2, 3, Point (vector2d (i - 0.4, 2.0 - 0.2), 1.0),
							  Point (vector2d (i + 0.4, 2.0 - 0.2), 1.0),
							  Point (vector2d (i, 2.0 + 0.4), 1.0));
}

/**
@brief Correct quit of application after main loop
*/
void destroy ()
{

}

/**
@brief process keyboard and mouse
@param window pointer to Window class that receives input events
*/
bool input (Window *window)
{
	static bool toogle_fullscreen = false;
	static bool is_move = false;
	static bool is_add = false;
	static vector2d old_center, mouse_center;
	static bool is_fixed = false;
	static std::vector<Point>::iterator fixed_point;
	
	// closing app
	if (keys[27])
		return false;

	// switch between fullscreen and windowed mode
	if (keys[(unsigned char)'f'] || keys[(unsigned char)'F'])
		toogle_fullscreen = true;
	else
		if (toogle_fullscreen)
		{
			window->Fullscreen ();
			toogle_fullscreen = !toogle_fullscreen;
		}

	// process scaling (mouse wheel)
	if (mouse.wheel == -1)
		camera.scale *= 1.25;
	else if (mouse.wheel == 1)
		camera.scale *= 0.8;
	mouse.wheel = 0;

	// process camera moving through right mouse button
	if (mouse.right)
	{
		if (is_move)
			camera.center = old_center + vector2d (2 * camera.aspect_ratio * camera.scale * (mouse_center.x - mouse.x) / camera.width,
												   2 * camera.scale * (mouse.y - mouse_center.y) / camera.height);
		else
		{
			is_move = true;
			old_center = camera.center;
			mouse_center = vector2d (mouse.x, mouse.y);
		}
	}
	else
		is_move = false;

	// process bodies drag and drop
	if (mouse.left)
	{
		if (is_fixed)
			fixed_point->cur_pos = camera.ToWorld (mouse.x, mouse.y);
		else
		{
			double min_sqr_len = 0.01;
			vector2d mouse_world = camera.ToWorld (mouse.x, mouse.y);
			for (size_t i = 0; i < world.DynamicBodies.size (); i++)
				for (size_t j = 0; j < world.DynamicBodies[i].points.size (); j++)
					if ((world.DynamicBodies[i].points[j].cur_pos - mouse_world).sqr_len () < min_sqr_len)
					{
						fixed_point = world.DynamicBodies[i].points.begin () + j;
						is_fixed = true;
					}
		}
	}
	else if (is_fixed)
		is_fixed = false;

	//add dynamic box
	if (keys[(unsigned char)' '])
		is_add = true;
	else if (is_add)
	{
		is_add = false;
		vector2d center = camera.ToWorld (mouse.x, mouse.y);
		world.AddDynamicBody (0.2, 4, Point (center + vector2d (-0.25, -0.25), 1.0),
							  Point (center + vector2d (0.25, -0.25), 1.0),
							  Point (center + vector2d (0.25, 0.25), 1.0),
							  Point (center + vector2d (-0.25, 0.25), 1.0));
	}
		

	return true;
}

/**
@brief renders objects that pass screen culling
*/
void render ()
{
	glClear (GL_COLOR_BUFFER_BIT);
	
	size_t drawed_static = 0, drawed_dynamic = 0;
	//Draw static objects
	glColor3f (1.0, 0.0, 0.0);
	for (size_t i = 0; i < world.StaticBodies.size (); i++)
		if (world.StaticBodies[i].bbox * camera.view_field)
		{
			drawed_static++;
			//draw figure
			glBegin (GL_LINE_LOOP);
			for (size_t j = 0; j < world.StaticBodies[i].points.size (); j++)
				glVertex2d (world.StaticBodies[i].points[j].x,
							world.StaticBodies[i].points[j].y);
			glEnd ();
		}

	//Draw dynamic objects
	for (size_t i = 0; i < world.DynamicBodies.size (); i++)
		if (world.DynamicBodies[i].bbox * camera.view_field)
		{
			drawed_dynamic++;
			//draw poles
			glColor3f (0.5, 0.5, 0.5);
			glBegin (GL_LINES);
			for (size_t j = 0; j < world.DynamicBodies[i].poles.size (); j++)
			{
				glVertex2d (world.DynamicBodies[i].poles[j].p1->cur_pos.x,
							world.DynamicBodies[i].poles[j].p1->cur_pos.y);
				glVertex2d (world.DynamicBodies[i].poles[j].p2->cur_pos.x,
							world.DynamicBodies[i].poles[j].p2->cur_pos.y);
			}
			glEnd ();

			//draw figure
			glColor3f (0.0, 1.0, 0.0);
			glBegin (GL_LINE_LOOP);
			for (size_t j = 0; j < world.DynamicBodies[i].points.size (); j++)
				glVertex2d (world.DynamicBodies[i].points[j].cur_pos.x,
							world.DynamicBodies[i].points[j].cur_pos.y);
			glEnd ();

			//draw vertexes
			glColor3f (1.0, 1.0, 1.0);
			glBegin (GL_POINTS);
			for (size_t j = 0; j < world.DynamicBodies[i].points.size (); j++)
				glVertex2d (world.DynamicBodies[i].points[j].cur_pos.x,
							world.DynamicBodies[i].points[j].cur_pos.y);
			glEnd ();
		}

	// Print statistics
	char str[50];
	sprintf (str, "Objects static: %u; dynamic: %u", world.StaticBodies.size (), world.DynamicBodies.size ());
	Print ((unsigned char *)str, 10, 40, 1.0, 1.0, 1.0);
	sprintf (str, "Drawed static: %u; dynamic: %u", drawed_static, drawed_dynamic);
	Print ((unsigned char *)str, 10, 60, 1.0, 1.0, 1.0);
}

int main (int argc, char *argv[])
{
	Window window (&argc, argv, 1280, 720, "Physics demo");
	init ();

	while (true)
	{
		int oldTime = glutGet (GLUT_ELAPSED_TIME);
		window.Process ();
		if (!input (&window))
			break;

		window.SetMatrix ();
		world.Update (10);
		render ();
		char str[50];
		sprintf (str, "Time in ms: %d", glutGet (GLUT_ELAPSED_TIME) - oldTime);
		while (glutGet (GLUT_ELAPSED_TIME) - oldTime < timestep * 1000);
		Print ((unsigned char *)str, 10, 20, 1.0, 1.0, 1.0);
		window.Present ();
	}
	destroy ();
	return 0;
}