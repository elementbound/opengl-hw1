#include "catchit.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include "glwrap/meshutil.h"
#include "glwrap/texture_util.h"
#include "glwrap/util.h"

#include <iostream>
#include <fstream>
#include <cstdlib> //std::exit

#define dieret(msg, val) {std::cerr << msg << std::endl; return val;}
#define die(msg) {std::cerr << msg << std::endl; std::exit(1);}
#define gldbg(msg) {GLenum err = glGetError(); std::cout << msg << gl_error_str(err) << '\n';}

//Not strictly for existence, more like for accessibility
//Which I basically need this for, so move on... 
bool file_exists(const char* fname)
{
	std::ifstream fs(fname);
	return fs.good();
}

bool app_CatchIt::glew_init() 
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

bool app_CatchIt::gl_init() 
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	gldbg("After init");
	
	return 1;
}

bool app_CatchIt::load_resources()
{
	std::vector<const char*> file_list = 
	{
		"data/skybox/alps_lf.png",
		"data/skybox/alps_rt.png",
		"data/skybox/alps_bk.png",
		"data/skybox/alps_ft.png",
		"data/skybox/alps_dn.png",
		"data/skybox/alps_up.png",
		
		"data/sphere.png",
		
		"data/skybox.vs",
		"data/skybox.fs",
		"data/textured.vs",
		"data/textured.fs",
		
		"data/sky.obj",
		"data/sphere.obj"
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
	std::cout << "Loading skybox... ";
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_POSITIVE_X, texutil::load_png("data/skybox/alps_lf.png"), GL_RGB);
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, texutil::load_png("data/skybox/alps_rt.png"), GL_RGB);
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, texutil::load_png("data/skybox/alps_bk.png"), GL_RGB);
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, texutil::load_png("data/skybox/alps_ft.png"), GL_RGB);
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, texutil::load_png("data/skybox/alps_dn.png"), GL_RGB);
		texture_Skybox.upload(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, texutil::load_png("data/skybox/alps_up.png"), GL_RGB);
		
		texture_Skybox.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		texture_Skybox.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		texture_Skybox.generate_mipmaps();
	std::cout << "done\n";
	
	std::cout << "Loading sphere texture... ";
		texture_Sphere.upload(texutil::load_png("data/sphere.png"), GL_RGB);
		
		texture_Sphere.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		texture_Sphere.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		texture_Sphere.generate_mipmaps();
	std::cout << "done\n";
	
	gldbg("Textures loaded");
	
	//Shaders
	std::cout << "Compiling skybox shader... ";
		shader_Skybox.create();
		
		if(!shader_Skybox.attach(read_file("data/skybox.vs").c_str(), shader_program::shader_type::vertex))
			dieret("\nCouldn't attach vertex shader", 0);
		
		if(!shader_Skybox.attach(read_file("data/skybox.fs").c_str(), shader_program::shader_type::fragment))
			dieret("\nCouldn't attach fragment shader", 0);
		
		glBindFragDataLocation(shader_Skybox.handle(), 0, "outColor");
		shader_Skybox.link();
	std::cout << "done\n";
	
	std::cout << "Compiling textured shader... ";
		shader_Textured.create();
		
		if(!shader_Textured.attach(read_file("data/textured.vs").c_str(), shader_program::shader_type::vertex))
			dieret("\nCouldn't attach vertex shader", 0);
		
		if(!shader_Textured.attach(read_file("data/textured.fs").c_str(), shader_program::shader_type::fragment))
			dieret("\nCouldn't attach fragment shader", 0);
		
		glBindFragDataLocation(shader_Textured.handle(), 0, "outColor");
		shader_Textured.link();
	std::cout << "done\n";
	
	gldbg("Shaders loaded");
	
	//Meshes
	std::cout << "Loading sphere mesh... ";
		meshutil::load_obj("data/sphere.obj", mesh_Sphere);
		shader_Textured.use();
		mesh_Sphere.bind();
	std::cout << "done\n";
	
	std::cout << "Loading skybox mesh... ";
		meshutil::load_obj("data/sky.obj", mesh_Skybox);
		shader_Skybox.use();
		mesh_Skybox.bind();
	std::cout << "done\n";
	
	gldbg("Meshes loaded");
	
	return 1;
}

void app_CatchIt::on_open()
{
	if(!glew_init())
		die("GLEW init fail");
	
	if(!gl_init())
		die("GL init fail");
	
	if(!load_resources())
		die("Couldn't load resources");
	
	//Init scene
	world_Player.position = glm::vec3(0.0f);
	world_Player.orientation = glm::vec3(0.0f);
	world_Player.velocity = glm::vec3(0.0f);
	
	for(int i=0; i<10; i++)
	{
		entity e;
			e.position = glm::ballRand(world_Radius * 0.75f);
			e.orientation = glm::vec3(0.0f);
			e.velocity = glm::sphericalRand(1.0f);
			
		world_Food.push_back(e);
	}
	
	//Initial resize ( window init kinda )
	{
		int w, h;
		glfwGetWindowSize(this->handle(), &w, &h);
		this->on_resize(w,h);
	}
}

void app_CatchIt::on_resize(int w, int h)
{
	resizable_window::on_resize(w,h);
	mat_Projection = glm::perspective(camera_FOV, (float)m_WindowAspect, 0.0625f, 8192.0f);
}

void app_CatchIt::on_key(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		std::cout << "Switching cameras... \n";
		camera_Mode = !camera_Mode;
	}
}

void app_CatchIt::update() 
{
	static double lastTime = 0;
	double newTime = glfwGetTime();
	double deltaTime = newTime - lastTime;
	lastTime = newTime;
	
	for(entity& e : world_Food)
	{
		e.position += float(deltaTime) * e.velocity;
		e.position = clamp(e.position, glm::vec3(-world_Radius+1.0f), glm::vec3(world_Radius-1.0f));
		if(e.position.x-1.0f < -world_Radius) e.velocity.x =  std::abs(e.velocity.x);
		if(e.position.x+1.0f >  world_Radius) e.velocity.x = -std::abs(e.velocity.x);
		if(e.position.y-1.0f < -world_Radius) e.velocity.y =  std::abs(e.velocity.y);
		if(e.position.y+1.0f >  world_Radius) e.velocity.y = -std::abs(e.velocity.y);
		if(e.position.z-1.0f < -world_Radius) e.velocity.z =  std::abs(e.velocity.z);
		if(e.position.z+1.0f >  world_Radius) e.velocity.z = -std::abs(e.velocity.z);
	}
	
	if(glfwGetKey(this->handle(), GLFW_KEY_RIGHT))
		world_Player.orientation.z += 0.25 * deltaTime;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_LEFT))
		world_Player.orientation.z -= 0.25 * deltaTime;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_UP))
		world_Player.orientation.x -= 0.25 * deltaTime;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_DOWN))
		world_Player.orientation.x += 0.25 * deltaTime;
	
	glm::vec3 forward = dirvec(world_Player.orientation.z, world_Player.orientation.x) * 3.0f * (float)deltaTime;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_Q))
		world_Player.position += forward;
	
	if(glfwGetKey(this->handle(), GLFW_KEY_W))
		world_Player.position -= forward;
	
	//AIMHACKZ CHEATER
	if(glfwGetKey(this->handle(), GLFW_KEY_SPACE) && !world_Food.empty())
	{
		forward = normalize(world_Food[0].position - world_Player.position) * 4.0f * (float)deltaTime;
		world_Player.position += forward;
	}
	
	world_Player.position = clamp(world_Player.position, glm::vec3(-world_Radius+2.0f), glm::vec3(world_Radius-2.0f));
	
	for(unsigned i=0; i<world_Food.size(); i++)
	{
		entity& e = world_Food[i];
		
		if(glm::length(e.position - world_Player.position) < 2.0f)
		{
			world_Food.erase(world_Food.begin() + i);
			i = 0;
			
			std::cout << "Remaining food: " << world_Food.size() << std::endl;
		}
	}
	
	if(world_Food.empty())
	{
		std::cout << "Game over, congrats\n";
		std::exit(0);
	}
}

void app_CatchIt::on_refresh()
{
	update();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	
	static glm::mat4 mat_World;
	static glm::mat4 mat_View;
	
	world_Player.calculateView();
	if(!camera_Mode)
		mat_View = world_Player.transform(); 
	else
		mat_View = glm::lookAt(glm::vec3(world_Radius - 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	
	//Draw sky
	shader_Skybox.use();
	texture_Skybox.use();
	mat_World = glm::scale(glm::mat4(), glm::vec3(world_Radius));
	
	shader_Skybox.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
	mesh_Skybox.draw();
	
	//Draw food
	shader_Textured.use();
	shader_Textured.set_uniform("uAmbient", glm::vec3(0.5));
	shader_Textured.set_uniform("uLightColor", glm::vec3(0.75));
	shader_Textured.set_uniform("uLightPos", glm::vec3(0.0, 0.0, world_Radius * 1.025f));
	
	texture_Sphere.use();
	for(entity& e : world_Food)
	{
		e.calculateTransform();
		mat_World = e.transform();
		
		shader_Textured.set_uniform("uModel", mat_World);
		shader_Textured.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		mesh_Sphere.draw();
	}
	
	if(camera_Mode)
	{
		world_Player.calculateTransform();
		mat_World = world_Player.transform();
		mat_World = glm::scale(mat_World, glm::vec3(2.0f));
		
		shader_Textured.set_uniform("uModel", mat_World);
		shader_Textured.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		mesh_Sphere.draw();
	}
	
	glfwSwapBuffers(this->handle());
			
	if(glfwGetKey(this->handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(this->handle(), 1);
}