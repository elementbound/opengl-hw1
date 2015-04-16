#include "catchit.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glwrap/meshutil.h"
#include "glwrap/texture_util.h"
#include "glwrap/util.h"

#include <fstream>
#include <cstdlib> //std::exit

#define dieret(msg, val) {std::cerr << msg << std::endl; return val;}
#define die(msg) {std::cerr << msg << std::endl; std::exit(1);}

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
		"data/phong.vs",
		"data/phong.fs",
		
		"data/cube.obj",
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
	
	std::cout << "Compiling phong shader... ";
		shader_Phong.create();
		
		if(!shader_Phong.attach(read_file("data/phong.vs").c_str(), shader_program::shader_type::vertex))
			dieret("\nCouldn't attach vertex shader", 0);
		
		if(!shader_Phong.attach(read_file("data/phong.fs").c_str(), shader_program::shader_type::fragment))
			dieret("\nCouldn't attach fragment shader", 0);
		
		glBindFragDataLocation(shader_Phong.handle(), 0, "outColor");
		shader_Phong.link();
	std::cout << "done\n";
	
	//Meshes
	std::cout << "Loading skybox mesh... ";
		meshutil::load_obj("data/cube.obj", mesh_Skybox);
		shader_Skybox.use();
		mesh_Skybox.bind();
	std::cout << "done\n";
	
	std::cout << "Loading sphere mesh... ";
		meshutil::load_obj("data/sphere.obj", mesh_Sphere);
		shader_Phong.use();
		mesh_Sphere.bind();
	std::cout << "done\n";
	
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
			e.position = glm::ballRand(16.0f);
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
	mat_Projection = glm::perspective(camera_FOV, (float)m_WindowAspect, 1.0f, 8192.0f);
}

void app_CatchIt::update() 
{
	static double lastTime = 0;
	double newTime = glfwGetTime();
	double deltaTime = newTime - lastTime;
	lastTime = newTime;
	
	for(entity& e : world_Food)
		e.position += float(deltaTime) * e.velocity;
}

void app_CatchIt::on_refresh()
{
	update();
	
	static glm::mat4 mat_World;
	static glm::mat4 mat_View;
	
	world_Player.calculateView();
	mat_View = world_Player.transform(); //glm::lookAt(glm::vec3(0.0f), world_Food[0].position, glm::vec3(0.0f,0.0f,1.0f));

	//
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Draw sky
	glDisable(GL_CULL_FACE);
	shader_Skybox.use();
	texture_Skybox.use();
	mat_World = glm::mat4();
	glm::scale(mat_World, glm::vec3(2.0f));
	glm::translate(mat_World, world_Player.position);
	
	shader_Skybox.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
	mesh_Skybox.draw();
	
	glEnable(GL_CULL_FACE);
	
	//Draw food
	shader_Phong.use();
	texture_Sphere.use();
	for(entity& e : world_Food)
	{
		e.calculateTransform();
		mat_World = e.transform();
		
		shader_Phong.set_uniform("uMVP", mat_Projection * mat_View * mat_World);
		mesh_Sphere.draw();
	}
	
	glfwSwapBuffers(this->handle());
			
	if(glfwGetKey(this->handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(this->handle(), 1);
}