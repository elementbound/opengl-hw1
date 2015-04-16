#include "catchit.h"

#include "glwrap/meshutil.h"
#include "glwrap/texture_util.h"
#include "glwrap/util.h"

#include <fstream>

#define dieret(msg, val) {std::cerr << msg; return val;}

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
	
	//Meshes
	std::cout << "Loading skybox mesh... ";
		meshutil::load_obj("data/cube.obj", mesh_Skybox);
	std::cout << "done\n";
	
	std::cout << "Loading sphere mesh... ";
		meshutil::load_obj("data/sphere.obj", mesh_Sphere);
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
}