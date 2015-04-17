#include "bounceit.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include "glwrap/meshutil.h"
#include "glwrap/texture_util.h"
#include "glwrap/util.h"

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib> //std::exit, 
#include <cmath>
#include <vector>

#define dieret(msg, val) {std::cerr << msg << std::endl; return val;}
#define die(msg) {std::cerr << msg << std::endl; std::exit(1);}
#define gldbg(msg) {GLenum err = glGetError(); std::cout << msg << gl_error_str(err) << '\n';}

//My current version of mingw doesn't find std::to_string, so I'll just leave this here
template < typename T > std::string to_string( const T& n )
{
	std::ostringstream stm ;
	stm << n ;
	return stm.str() ;
}

//Not strictly for existence, more like for accessibility
//Which I basically need this for, so move on... 
bool file_exists(const char* fname)
{
	std::ifstream fs(fname);
	return fs.good();
}

bool app_BounceIt::glew_init() 
{
	this->make_current();
	
	if(glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW init fail" << std::endl;
		return 0;
	}

	if(!GLEW_VERSION_4_4)
	{
		std::cerr << "OpenGL 4.4 not supported" << std::endl;
		return 0; 
	}
	
	return 1;
}

bool app_BounceIt::gl_init() 
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	gldbg("After init");
	
	return 1;
}

bool app_BounceIt::load_resources()
{
	std::vector<const char*> file_list = 
	{
		"data/bricks.png",
		"data/concrete.png",
		"data/beachball.png",
		
		"data/cube.obj",
		"data/sphere.obj",
		
		"data/phong.vs",
		"data/phong.fs"
	};
	
	bool fail = false;
	for(const char* fname : file_list) 
		if(!file_exists(fname))
		{
			std::cerr << '\"' << fname << "\" missing!\n";
			fail = true;
		}
		
	if(fail) 
		return 0;
	
	//Textures
	#define SETUP_TEXTURE(texture) { \
		texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); \
		texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR); \
		texture.generate_mipmaps(); \
	}
	
	std::cout << "Loading textures... ";
		texture_Background.upload(texutil::load_png("data/bricks.png"), GL_RGB);
		texture_Box.upload(texutil::load_png("data/concrete.png"), GL_RGB);
		texture_Sphere.upload(texutil::load_png("data/beachball.png"), GL_RGB);
		
		SETUP_TEXTURE(texture_Background);
		SETUP_TEXTURE(texture_Box);
		SETUP_TEXTURE(texture_Sphere);
	std::cout << "done\n";
	
	#undef SETUP_TEXTURE
	
	//Shaders
	std::cout << "Compiling phong shader... ";
		shader_Phong.create();
		
		if(!shader_Phong.attach(read_file("data/phong.vs").c_str(), shader_program::shader_type::vertex))
			dieret("\nCouldn't attach vertex shader", 0);
		
		if(!shader_Phong.attach(read_file("data/phong.fs").c_str(), shader_program::shader_type::fragment))
			dieret("\nCouldn't attach fragment shader", 0);
		
		glBindFragDataLocation(shader_Phong.handle(), 0, "outColor");
		shader_Phong.link();
	std::cout << "done\n";
	
	gldbg("Shaders loaded");
	
	//Meshes
	std::cout << "Loading box mesh... ";
		meshutil::load_obj("data/cube.obj", mesh_Box);
		shader_Phong.use();
		mesh_Box.bind();
	std::cout << "done\n";
	
	std::cout << "Loading sphere mesh... ";
		meshutil::load_obj("data/sphere.obj", mesh_Sphere);
		shader_Phong.use();
		mesh_Sphere.bind();
	std::cout << "done\n";
	
	gldbg("Meshes loaded");
	
	return 1;
}

void app_BounceIt::on_open()
{
	if(!glew_init())
		die("GLEW init fail");
	
	if(!gl_init())
		die("GL init fail");
	
	if(!load_resources())
		die("Couldn't load resources");
	
	//Init scene
	world_Sphere.position = glm::vec3(0.0f);
	world_Sphere.velocity = glm::sphericalRand(1.0f);
	
	world_Box.position = glm::vec3(-world_Radius + 1.0f);
	
	spotlight_Pos[0] = glm::vec3( world_Radius,  world_Radius,  world_Radius);
	spotlight_Pos[1] = glm::vec3(-world_Radius,  world_Radius,  world_Radius);
	spotlight_Pos[2] = glm::vec3( world_Radius, -world_Radius,  world_Radius);
	spotlight_Pos[3] = glm::vec3(-world_Radius, -world_Radius,  world_Radius);
	
	pointlight_Pos = glm::vec3(0.0f);
	
	camera_Pos = spotlight_Pos[0];
	
	//Initial resize ( window init kinda )
	{
		int w, h;
		glfwGetWindowSize(this->handle(), &w, &h);
		this->on_resize(w,h);
	}
}

void app_BounceIt::on_resize(int w, int h)
{
	resizable_window::on_resize(w,h);
	mat_Projection = glm::perspective(camera_FOV, (float)m_WindowAspect, 0.0625f, 8192.0f);
}

void app_BounceIt::update()
{
	static double lastTime = 0;
	double newTime = glfwGetTime();
	double deltaTime = newTime - lastTime;
	lastTime = newTime;
	
	world_Sphere.position += world_Sphere.velocity * (float)deltaTime;
	
	if(world_Sphere.position.x-1.0f < -world_Radius) world_Sphere.velocity.x =  std::abs(world_Sphere.velocity.x);
	if(world_Sphere.position.x+1.0f >  world_Radius) world_Sphere.velocity.x = -std::abs(world_Sphere.velocity.x);
	if(world_Sphere.position.y-1.0f < -world_Radius) world_Sphere.velocity.y =  std::abs(world_Sphere.velocity.y);
	if(world_Sphere.position.y+1.0f >  world_Radius) world_Sphere.velocity.y = -std::abs(world_Sphere.velocity.y);
	if(world_Sphere.position.z-1.0f < -world_Radius) world_Sphere.velocity.z =  std::abs(world_Sphere.velocity.z);
	if(world_Sphere.position.z+1.0f >  world_Radius) world_Sphere.velocity.z = -std::abs(world_Sphere.velocity.z);
	
	/*world_Sphere.position -= world_Box.position;
	if(world_Sphere.position.x-1.0f < -1.0f) world_Sphere.velocity.x = -std::abs(world_Sphere.velocity.x);
	if(world_Sphere.position.x+1.0f >  1.0f) world_Sphere.velocity.x = +std::abs(world_Sphere.velocity.x);
	if(world_Sphere.position.y-1.0f < -1.0f) world_Sphere.velocity.y = -std::abs(world_Sphere.velocity.y);
	if(world_Sphere.position.y+1.0f >  1.0f) world_Sphere.velocity.y = +std::abs(world_Sphere.velocity.y);
	if(world_Sphere.position.z-1.0f < -1.0f) world_Sphere.velocity.z = -std::abs(world_Sphere.velocity.z);
	if(world_Sphere.position.z+1.0f >  1.0f) world_Sphere.velocity.z = +std::abs(world_Sphere.velocity.z);
	
	world_Sphere.position += world_Box.position;*/
	
	//
	
	float box_speed = 1.0f * deltaTime;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_W))
		world_Box.position.y += box_speed;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_S))
		world_Box.position.y -= box_speed;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_D))
		world_Box.position.x += box_speed;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_A))
		world_Box.position.x -= box_speed;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_E))
		world_Box.position.z += box_speed;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_Q))
		world_Box.position.z -= box_speed;
}

void app_BounceIt::on_refresh()
{
	update();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	static glm::mat4 mat_World;
	static glm::mat4 mat_View;
	static glm::mat4 mat_Normal;
	
	mat_Normal = glm::mat4();
	mat_View = glm::lookAt(camera_Pos, world_Sphere.position, glm::vec3(0.0f, 0.0f, 1.0f));
	
	shader_Phong.use();
		shader_Phong.set_uniform("uLights[0].type", 2);
		shader_Phong.set_uniform("uLights[0].position", pointlight_Pos);
		
		for(unsigned i=0; i<4; i++)
		{
			std::string name = "uLights[" + to_string(i+1) + "]";
			glm::vec3 direction = glm::normalize(glm::vec3(0.0f) - spotlight_Pos[i]);
			
			shader_Phong.set_uniform((name + ".type").c_str(), 1);
			shader_Phong.set_uniform((name + ".position").c_str(), spotlight_Pos[i]);
			shader_Phong.set_uniform((name + ".direction").c_str(), direction);
			shader_Phong.set_uniform((name + ".thresh").c_str(), std::cos(spotlight_Angle));
		}
		
		//Draw background/room
		texture_Background.use();
		mat_World = glm::scale(glm::mat4(), glm::vec3(-world_Radius));
		mat_Normal = glm::scale(glm::mat4(), glm::vec3(-1.0f));
		shader_Phong.set_uniform("uModel", mat_World);
		shader_Phong.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		shader_Phong.set_uniform("uNormalMatrix", mat_Normal);
		mesh_Box.draw();
		
		mat_Normal = glm::mat4();
		
		//Draw box
		texture_Box.use();
		mat_World = glm::translate(glm::mat4(), glm::vec3(world_Box.position));
		shader_Phong.set_uniform("uModel", mat_World);
		shader_Phong.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		shader_Phong.set_uniform("uNormalMatrix", mat_Normal);
		mesh_Box.draw();
		
		//Draw sphere
		texture_Sphere.use();
		mat_World = glm::translate(glm::mat4(), glm::vec3(world_Sphere.position));
		shader_Phong.set_uniform("uModel", mat_World);
		shader_Phong.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		shader_Phong.set_uniform("uNormalMatrix", mat_Normal);
		mesh_Sphere.draw();
			
	glfwSwapBuffers(this->handle());
			
	if(glfwGetKey(this->handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(this->handle(), 1);
}