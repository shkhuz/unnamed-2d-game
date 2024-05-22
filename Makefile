SRCS := \
	imgui/imgui_widgets.cpp \
	imgui/imgui_tables.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui_demo.cpp \
	imgui/imgui.cpp \
	imgui/imgui_stdlib.cpp \
	imgui/imgui_impl_sdl2.cpp \
	imgui/imgui_impl_opengl3.cpp \
	src/main.cpp

OBJS := $(addprefix build/obj/, $(addsuffix .o, $(SRCS)))

INCLUDES := -I/usr/include/SDL2
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

