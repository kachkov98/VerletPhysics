/**
@brief implementation of scene loader
@author Sergei Kachkov
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "loader.h"
#include "physics.h"
#include "log.h"

bool is_valid (const char *str)
{
	for (int i = 0; i < strlen (str); i++)
	{
		if (str[i] == '%')
			return false;
		else if (!isspace (str[i]))
			return true;
	}
	return false;
}

bool next_str (FILE *f, char *str, size_t max_len)
{
	do
	{
		if (feof (f))
			return false;
		fgets (str, max_len, f);
	} while (!is_valid (str));
	return true;
}

void load_scene (Physics *engine, const char *path)
{
	FILE *f = fopen (path, "r");
	if (f)
		log (LOG_INFO, "scene file %s was opened successfully", path);
	else
		log (LOG_FAIL, "can not open scene file %s", path);

	char cur_str[256];
	while (next_str(f, cur_str, 256))
	{
		size_t num_points, id;
		sscanf (cur_str, "%u", &num_points);
		if (num_points < 3)
			log (LOG_FAIL, "number of points in scene file must be more than 2");

		vector2d p[3];
		for (size_t i = 0; i < 3; i++)
		{
			if (!next_str (f, cur_str, 256))
				log (LOG_FAIL, "number of points is not equal to defined number in description of body");
			sscanf (cur_str, "%lf %lf", &p[i].x, &p[i].y);
		}

		id = engine->AddStaticBody (3, p[0], p[1], p[2]);

		for (size_t i = 3; i < num_points; i++)
		{
			if (!next_str (f, cur_str, 256))
				log (LOG_FAIL, "number of points is not equal to defined number in description of body");
			vector2d p;
			sscanf (cur_str, "%lf %lf", &p.x, &p.y);
			engine->StaticBodies[id].AddStaticPoint (p);
		}
	}
	log (LOG_INFO, "scene have loaded successfully");
}