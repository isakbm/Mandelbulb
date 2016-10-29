IDIRS=-IC:/Libraries/glfw-3.2.1.bin.WIN64/include -IC:/Libraries/glew-2.0.0/include
LDIRS=-LC:/Libraries/glfw-3.2.1.bin.WIN64/lib-mingw-w64 -LC:/Libraries/glew-2.0.0/lib
	
LFLAGS= -lglew32 -lglfw3 -lopengl32 -lgdi32

all:
	g++ -DGLEW_STATIC main.cpp controller.cpp $(IDIRS) $(LDIRS) $(LFLAGS)