#ifndef _H_CATCHIT_H_
#define _H_CATCHIT_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

#include "glwrap/window.h"
#include "glwrap/resizable_window.h"
#include "glwrap/shader.h"
#include "glwrap/mesh.h"
#include "glwrap/texture.h"

#include "entity.h"

class app_CatchIt : public resizable_window
{
	private: 
		//Resources
		separated_mesh		mesh_Skybox;
		separated_mesh		mesh_Sphere;
		
		shader_program		shader_Skybox;
		shader_program 		shader_Textured;
		
		texture_cube		texture_Skybox;
		texture2d			texture_Sphere;
		
		//World
		std::vector<entity>	world_Food;
		entity				world_Player;
		float 				world_Radius = 16.0f;
		
		bool				camera_Mode = 0;
		float				camera_FOV = glm::radians(60.0f);
		glm::mat4			mat_Projection;
		
		void update();
		
		//etc
		bool glew_init();
		bool gl_init();
		bool load_resources();
		
	protected: 
		void on_open();
		void on_resize(int w, int h);
		void on_refresh();
		
		void on_key(int key, int scancode, int action, int mods);
};

#endif