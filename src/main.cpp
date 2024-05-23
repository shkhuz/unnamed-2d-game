#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stdio.h>
#include <chrono>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

#include "io.cpp"
#include "shader.cpp"
#include "camera.cpp"

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        360,
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    printf("%s\n", glGetString(GL_VERSION));
    SDL_GL_SetSwapInterval(0);

    glewExperimental = true;
    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(window, glctx);
    ImGui_ImplOpenGL3_Init();

    SDL_Surface* png = IMG_Load("res/img.png");
    if (!png) {
        fprintf(stderr, "Unable to load png: %s\n", IMG_GetError());
        return 1;
    }

    float size = 350.0f;
    glm::vec2 quads[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(500.0f, 0.0f),
        glm::vec2(1000.0f, 0.0f),
        glm::vec2(1000.0f, 500.0f),
    };
    int num_quads = sizeof(quads)/sizeof(glm::vec2);
    printf("num quads: %d\n", num_quads);

    const glm::vec2 VERTEX_OFFSET_LOOKUP[4] = {
        glm::vec2(0.0f, size),
        glm::vec2(size, size),
        glm::vec2(size, 0.0f),
        glm::vec2(0.0f, 0.0f),
    };

    const glm::vec2 TEXCOORDS_LOOKUP[4] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
    };

    int sizeof_vertices = num_quads * 9 * 4 * sizeof(float);
    printf("sizeof_vertices: %d\n", sizeof_vertices);
    float* vertices = (float*)malloc(sizeof_vertices);
    for (int i = 0; i < num_quads; i++) {
        for (int j = 0; j < 4; j++) {
            int offset = i*9*4 + j*9;

            vertices[offset] =   quads[i].x + VERTEX_OFFSET_LOOKUP[j].x;
            vertices[offset+1] = quads[i].y + VERTEX_OFFSET_LOOKUP[j].y;
            vertices[offset+2] = 0.0f;

            vertices[offset+3] = 1.0f;
            vertices[offset+4] = 1.0f;
            vertices[offset+5] = 1.0f;
            vertices[offset+6] = 1.0f;

            vertices[offset+7] = TEXCOORDS_LOOKUP[j].x;
            vertices[offset+8] = TEXCOORDS_LOOKUP[j].y;
        }
    }

    int sizeof_indices = num_quads * 6 * sizeof(int);
    printf("sizeof_indices: %d\n", sizeof_indices);
    int* indices = (int*)malloc(sizeof_indices);
    for (int i = 0; i < num_quads; i++) {
        int offset = i * 6;

        indices[offset]   = 3 + i*4;
        indices[offset+1] = 2 + i*4;
        indices[offset+2] = 1 + i*4;

        indices[offset+3] = 1 + i*4;
        indices[offset+4] = 0 + i*4;
        indices[offset+5] = 3 + i*4;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertices_vbo;
    glGenBuffers(1, &vertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof_vertices,
        vertices,
        GL_STATIC_DRAW
    );

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof_indices,
        indices,
        GL_STATIC_DRAW
    );

    int position_floats = 3;
    int color_floats = 4;
    int texcoord_floats = 2;

    int total_floats = position_floats + color_floats + texcoord_floats;
    int stride = total_floats*sizeof(float);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        0
    );
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)(position_floats*sizeof(float))
    );
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void*)((position_floats + color_floats)*sizeof(float))
    );
    glBindVertexArray(0);

    ShaderProgram shader;
    if (!shader.init_from_files(
        "res/shaders/sprite_vertex.glsl",
        "res/shaders/sprite_fragment.glsl")
    ) return 1;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        png->w,
        png->h,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        png->pixels
    );
    SDL_FreeSurface(png);

    Camera camera;
    camera.set_proj();
    camera.set_view();

    auto begin_time = std::chrono::high_resolution_clock::now();
    auto end_time = begin_time;
    float dt = -1.0f;

    const float CAM_SPEED = 400.0f;
    glm::vec2 camvel = glm::vec2();

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch (e.key.keysym.sym) {
                    case SDLK_a: camvel.x = -CAM_SPEED; break;
                    case SDLK_d: camvel.x =  CAM_SPEED;  break;
                    case SDLK_w: camvel.y =  CAM_SPEED;  break;
                    case SDLK_s: camvel.y = -CAM_SPEED; break;
                    case SDLK_q: camera.pos = glm::vec2(); break;
                    case SDLK_ESCAPE: running = false; break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_a:
                    case SDLK_d:
                        camvel.x = 0;
                        break;

                    case SDLK_w:
                    case SDLK_s:
                        camvel.y = 0;
                        break;
                }
            }
        }

        camera.pos += dt * camvel;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera.set_view();
        ImGui::Text("fps: %f", 1.0f/dt);
        ImGui::Text("dt: %f", dt);
        ImGui::Text("cam_pos: (%f, %f)", camera.pos.x, camera.pos.y);

        shader.use();
        shader.upload_mat4("proj", camera.proj);
        shader.upload_mat4("view", camera.view);
        glBindVertexArray(vao);
        for (int i = 0; i < 1; i++)
            glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        end_time = std::chrono::high_resolution_clock::now();
        dt =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time)
            .count() / 1e9;
        begin_time = end_time;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    IMG_Quit();
    SDL_Quit();
}
