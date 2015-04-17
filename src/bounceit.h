#ifndef _H_BOUNCEIT_H_
#define _H_BOUNCEIT_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "glwrap/window.h"
#include "glwrap/resizable_window.h"
#include "glwrap/shader.h"
#include "glwrap/mesh.h"
#include "glwrap/texture.h"

#include "entity.h"

class app_BounceIt : public resizable_window
{
	private: 
		//Resources
		separated_mesh		mesh_Box;
		separated_mesh		mesh_Sphere;
		
		shader_program		shader_Phong;
		
		texture2d			texture_Background;
		texture2d			texture_Box;
		texture2d			texture_Sphere;
		
		//World
		float				world_Radius = 8.0f;
		entity 				world_Sphere;
		entity				world_Box;
		
		//Camera
		glm::vec3			camera_Pos;
		float				camera_FOV = glm::radians(60.0f);
		glm::mat4			mat_Projection;
		
		//Lights
		glm::vec3			spotlight_Pos[4];
		glm::vec3			pointlight_Pos;
		float				spotlight_Angle = glm::radians(30.0f);
		
		//etc
		bool glew_init();
		bool gl_init();
		bool load_resources();
		
		void update();
		
		protected: 
		void on_open();
		void on_resize(int w, int h);
		void on_refresh();
};

#endif