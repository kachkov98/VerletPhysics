/**
@file
@brief implementation of application framework
@author Sergei Kachkov
*/
#include <assert.h>
#include "window.h"
#include "log.h"

Camera camera;
bool is_close;

bool keys[256];
Mouse mouse;

vector2d Camera::ToWorld (int x, int y)
{
	assert (width && height);
	return vector2d (center.x - scale * aspect_ratio * (1 - x * 2 / (double) width),
					 center.y - scale * (1 - (height - y) * 2 / (double) height));
}

void Camera::SetMatrix ()
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glViewport (0, 0, camera.width, camera.height);
	camera.view_field.lb = vector2d (camera.center.x - camera.scale * camera.aspect_ratio,
									 camera.center.y - camera.scale);
	camera.view_field.rt = vector2d (camera.center.x + camera.scale * camera.aspect_ratio,
									 camera.center.y + camera.scale);
	gluOrtho2D (camera.view_field.lb.x, camera.view_field.rt.x,
				camera.view_field.lb.y, camera.view_field.rt.y);
}

//-------------------------------Callback functions------------------------------

/* this function doesn't do any important, because application forcedly redraw window in loop; 
   however, freeglut gives error without it */ 
void display ()
{
	return;
}

void reshape (int window_width, int window_height)
{
	assert (window_width && window_height);
	camera.width = window_width;
	camera.height = window_height;
	camera.aspect_ratio = (double)camera.width / (double)camera.height;
}

void close ()
{
	is_close = true;
}

void key_down (unsigned char key, int x, int y)
{
	keys[key] = true;
}

void key_up (unsigned char key, int x, int y)
{
	keys[key] = false;
}

void mouse_move (int x, int y)
{
	mouse.x = x;
	mouse.y = y;
}

void mouse_press (int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			mouse.left = true;
		else
			mouse.left = false;
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
			mouse.middle = true;
		else
			mouse.middle = false;
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
			mouse.right = true;
		else
			mouse.right = false;
		break;
	}
}

void mouse_wheel (int wheel, int direction, int x, int y)
{
	mouse.wheel = direction;
}
//------------------------------------End of callback functions------------------

//-----------------implementation of windows class-------------------------------
Window::Window (int *argc, char *argv[], int window_width, int window_height, const char *window_title)
{
	//init camera
	camera.width = window_width;
	camera.height = window_height;
	is_close = false;
	camera.center = vector2d (0, 2.0);
	camera.scale = 5.0;
	//init keyboard
	for (int i = 0; i < 256; i++)
		keys[i] = false;

	//start logging
	InitLog ();

	//create window
	glutInit (argc, argv);
	glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize (camera.width, camera.height);
	handle = glutCreateWindow (window_title);

	//set callbacks
	glutDisplayFunc (display);
	glutReshapeFunc (reshape);
	glutCloseFunc (close);
	glutKeyboardFunc (key_down);
	glutKeyboardUpFunc (key_up);
	glutMotionFunc (mouse_move);
	glutPassiveMotionFunc (mouse_move);
	glutMouseFunc (mouse_press);
	glutMouseWheelFunc (mouse_wheel);
	log (LOG_INFO, "Window \"%s\" created succesfully", window_title);
}

Window::~Window ()
{
	glutDestroyWindow (handle);
	log (LOG_INFO, "Window destroyed");
	CloseLog ();
}

void Window::Process ()
{
	glutMainLoopEvent ();
}

void Window::SetTitle (const char *window_title)
{
	glutSetWindowTitle (window_title);
}

void Window::Fullscreen ()
{
	glutFullScreenToggle ();
	log (LOG_INFO, "Toogle between fullscreen and windowed mode");
}

void Window::Present ()
{
	glutSwapBuffers ();
	glFinish ();
}
//-----------------end of implementation of window class-------------------------

void Print (const unsigned char *str, int x, int y, GLfloat r, GLfloat g, GLfloat b)
{
	assert (str);
	glPushMatrix ();
	glPushClientAttrib (GL_CURRENT_BIT);
	glColor3f (r, g, b);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0, camera.width, camera.height, 0);
	glRasterPos2i (x, y);
	glutBitmapString (GLUT_BITMAP_9_BY_15, str);
	glPopClientAttrib ();
	glPopMatrix ();
}