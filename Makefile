CXX_FLAGS = -std=c++11 -I/
COMMON_DEPS = bin/window.o bin/resizable_window.o bin/shader.o bin/util.o bin/basic_mesh.o bin/separated_mesh.o bin/texture.o bin/texture_util.o
LINKER_SETTINGS = -lglfw3 -lgdi32 -lopengl32 -lglew32 -lpng -lz

part1:
	g++ ${CXX_FLAGS} -c src/entity.cpp -o bin/entity.o
	g++ ${CXX_FLAGS} -c src/catchit.cpp -o bin/catchit.o
	g++ ${CXX_FLAGS} -c src/part1.cpp -o bin/part1.o