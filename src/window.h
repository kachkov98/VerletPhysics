/**
@file
@brief header of application framework
@author Sergei Kachkov
*/
#pragma once
#include <GL/freeglut.h>
#include "vector.h"
#include "boundingbox.h"

/**
@class
@brief implements world movement and transformation of coordinates
top has coordinate center.y + scale
bottom has coordinate center.y - scale
left has coordinate center.x - scale * aspect_ratio
right has coordinate center.x + scale * aspect_ratio
left has coordinate
*/
struct Camera
{
	int width, height;
	double aspect_ratio;
	vector2d center;
	double scale;
	BoundingBox view_field;
	/**
	@brief transformate screen coordinates to world
	@param x x screen coordinate (left to right)
	@param y y screen coordinate (top to bottom)
	@return world right-sided coordinates
	*/
	vector2d ToWorld (int x, int y);
	/**
	@brief sets viewport and modelview matrix
	*/
	void SetMatrix ();
};
extern Camera camera;

extern bool is_close;

// array of current pressed keys
extern bool keys[256];
// state of mouse
struct Mouse
{
	bool left, middle, right;
	int x, y, wheel;
};
extern Mouse mouse;

/**
@brief function prints string in OpenGL window
@param str pointer to string
@param x, y coordinates of position where string will print
@param r, g, b color of string
*/
void Print (const unsigned char *str, int x, int y, GLfloat r, GLfloat g, GLfloat b);

/**
@class
@brief controls application's window
*/
class Window
{
private:
	int handle;
public:
	/**
	@brief creates window
	@param argc, argv command line arguments
	@param window_width, window_height size of window on the moment of creation
	@warning size of window can be changed during work of application
	@param window_title title of window
	@note it can be changed later by calling SetTitle (const char *title);
	*/
	Window (int *argc, char *argv[], int window_width, int window_height, const char *window_title);
	~Window ();
	/**
	@brief processes all window events
	*/
	void Process ();
	/**
	@brief changes title of window
	@param title new title of window
	*/
	void SetTitle (const char *title);
	/**
	@brief switches between windowed and fullscreen mode
	*/
	void Fullscreen ();
	/**
	@brief present back buffer and execute all deferred opengl commands
	*/
	void Present ();
};
