CXX_FLAGS = -std=c++11 -I/ -Os
GLWRAP_BINS = bin/window.o bin/resizable_window.o bin/shader.o bin/util.o bin/basic_mesh.o bin/separated_mesh.o bin/meshutil.o bin/texture.o bin/texture_util.o
OWN_BINS = bin/entity.o
LINKER_SETTINGS = -lglfw3 -lgdi32 -lopengl32 -lglew32 -lpng -lz

all: part1 part2

part1: glwrap deps
	g++ ${CXX_FLAGS} -c src/catchit.cpp -o bin/catchit.o
	g++ ${CXX_FLAGS} -c src/part1.cpp -o bin/part1.o
	g++ bin/part1.o bin/catchit.o ${OWN_BINS} ${GLWRAP_BINS} ${LINKER_SETTINGS} -o part1
	strip part1.exe
	
part2: glwrap deps
	g++ ${CXX_FLAGS} -c src/bounceit.cpp -o bin/bounceit.o
	g++ ${CXX_FLAGS} -c src/part2.cpp -o bin/part2.o
	g++ bin/part2.o bin/bounceit.o ${OWN_BINS} ${GLWRAP_BINS} ${LINKER_SETTINGS} -o part2
	strip part2.exe

deps: 
	g++ ${CXX_FLAGS} -c src/entity.cpp -o bin/entity.o
	
glwrap: 
	g++ ${CXX_FLAGS} -c src/glwrap/window.cpp -o bin/window.o
	g++ ${CXX_FLAGS} -c src/glwrap/resizable_window.cpp -o bin/resizable_window.o
	g++ ${CXX_FLAGS} -c src/glwrap/shader.cpp -o bin/shader.o
	g++ ${CXX_FLAGS} -c src/glwrap/util.cpp -o bin/util.o
	g++ ${CXX_FLAGS} -c src/glwrap/mesh/basic_mesh.cpp -o bin/basic_mesh.o
	g++ ${CXX_FLAGS} -c src/glwrap/mesh/separated_mesh.cpp -o bin/separated_mesh.o
	g++ ${CXX_FLAGS} -c src/glwrap/meshutil.cpp -o bin/meshutil.o
	g++ ${CXX_FLAGS} -c src/glwrap/texture.cpp -o bin/texture.o
	g++ ${CXX_FLAGS} -c src/glwrap/texture_util.cpp -o bin/texture_util.o
	
clean: 
	del bin\*.o