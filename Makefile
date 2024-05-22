SRCS := \
	thirdparty/imgui/imgui_widgets.cpp \
	thirdparty/imgui/imgui_tables.cpp \
	thirdparty/imgui/imgui_draw.cpp \
	thirdparty/imgui/imgui_demo.cpp \
	thirdparty/imgui/imgui.cpp \
	thirdparty/imgui/imgui_stdlib.cpp \
	thirdparty/imgui/imgui_impl_sdl2.cpp \
	thirdparty/imgui/imgui_impl_opengl3.cpp \
	src/main.cpp

OBJS := $(addprefix build/obj/, $(addsuffix .o, $(SRCS)))

INCLUDES := -I/usr/include/SDL2 -Ithirdparty
LIBS := -lSDL2 -lSDL2_image -lGLEW -lGL -lglm
FLAGS := -g -Wall -Wextra

run: $(OBJS) build/a.out
	./build/a.out

build/a.out: $(OBJS)
	g++ -o build/a.out $(FLAGS) $(OBJS) $(LIBS)

build/obj/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	g++ -c $^ $(FLAGS) -o $@ $(INCLUDES)

clean:
	rm -rf build/

.PHONY: clean

