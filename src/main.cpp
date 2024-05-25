#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

#include "io.cpp"
#include "shader.cpp"
#include "camera.cpp"

int WIDTH = 1280;
int HEIGHT = 720;
int FLAGS = SDL_WINDOW_OPENGL;

//int WIDTH = 1920;
//int HEIGHT = 1080;
//int FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

class Random {
public:
    static float real() {
        return (float)rand() / (float)(RAND_MAX/1.0f);
    }
};

class ChunkSlot {
public:
    static constexpr float TILE_SIZE = 16.0f;
    static const int ROWS = 32;

    static constexpr glm::vec2 VERTEX_OFFSET_LOOKUP[4] = {
        glm::vec2(0.0f,      TILE_SIZE),
        glm::vec2(TILE_SIZE, TILE_SIZE),
        glm::vec2(TILE_SIZE, 0.0f),
        glm::vec2(0.0f,      0.0f),
    };

    static const int STATIC_ATTR_LEN = 6;
    static const int DYNAMIC_ATTR_LEN = 2;

    glm::vec3 quad_data[ROWS * ROWS];

    float  static_vertex_data[ROWS*ROWS * 4 * STATIC_ATTR_LEN  * sizeof(float)];
    float dynamic_vertex_data[ROWS*ROWS * 4 * DYNAMIC_ATTR_LEN * sizeof(float)];
    GLuint vao, static_vbo, dynamic_vbo;

    void init_render_buffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &static_vbo);
        glGenBuffers(1, &dynamic_vbo);
    }

    void load_quad_data(int chunkX, int chunkY) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < ROWS; j++) {
                quad_data[j*ROWS+i] = glm::vec3(
                    i*TILE_SIZE,
                    j*TILE_SIZE,
                    Random::real()
                );
            }
        }
    }

    void fill_vertex_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            for (int v = 0; v < 4; v++) {
                int soffset = q*4*STATIC_ATTR_LEN + v*STATIC_ATTR_LEN;
                static_vertex_data[soffset]   = quad_data[q].x + VERTEX_OFFSET_LOOKUP[v].x;
                static_vertex_data[soffset+1] = quad_data[q].y + VERTEX_OFFSET_LOOKUP[v].y;

                static_vertex_data[soffset+2] = 1.0f;
                static_vertex_data[soffset+3] = 1.0f;
                static_vertex_data[soffset+4] = 1.0f;
                static_vertex_data[soffset+5] = 1.0f;

                int doffset = q*4*DYNAMIC_ATTR_LEN + v*DYNAMIC_ATTR_LEN;
                //dynamic_vertex_data[doffset] =
            }
        }
    }
};

const int CHUNKS = 9;
ChunkSlot* cslots = NULL;

int main() {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        FLAGS
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

    SDL_Surface* png = IMG_Load("res/atlas.jpg");
    if (!png) {
        fprintf(stderr, "Unable to load img: %s\n", IMG_GetError());
        return 1;
    }

    cslots = (ChunkSlot*)malloc(sizeof(ChunkSlot) * CHUNKS);

    Camera camera;

    float size = 16.0f;
    int quads_in_row = 100;
    int num_quads = quads_in_row * quads_in_row;
    printf("num_quads: %d\n", num_quads);
    glm::vec3* quads = (glm::vec3*)malloc(sizeof(glm::vec3)*num_quads);
    for (int i = 0; i < quads_in_row; i++) {
        for (int j = 0; j < quads_in_row; j++) {
            quads[j*(quads_in_row) + i] = glm::vec3(i*size, j*size, Random::real());
        }
    }
    //camera.pos.x = quads_in_row/2 * size;
    //camera.pos.y = quads_in_row/2 * size;

    const glm::vec2 VERTEX_OFFSET_LOOKUP[4] = {
        glm::vec2(0.0f,      size),
        glm::vec2(size, size),
        glm::vec2(size, 0.0f),
        glm::vec2(0.0f,      0.0f),
    };

    const float TEAL_TEXCOORDS_LOOKUP[] = {
        0.0f, 0.0f,
        0.5f, 0.0f,
        0.5f, 1.0f,
        0.0f, 1.0f,
    };

    const float DRY_TEXCOORDS_LOOKUP[] = {
        0.5f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.5f, 1.0f,
    };

    int sizeof_static_vertdata = num_quads * 7 * 4 * sizeof(float);
    printf("sizeof_static_vertdata: %d KB\n", sizeof_static_vertdata/1000);
    float* static_vertdata = (float*)malloc(sizeof_static_vertdata);
    for (int i = 0; i < num_quads; i++) {
        for (int j = 0; j < 4; j++) {
            int offset = i*7*4 + j*7;

            static_vertdata[offset] =   quads[i].x + VERTEX_OFFSET_LOOKUP[j].x;
            static_vertdata[offset+1] = quads[i].y + VERTEX_OFFSET_LOOKUP[j].y;
            static_vertdata[offset+2] = 0.0f;

            static_vertdata[offset+3] = 1.0f;
            static_vertdata[offset+4] = 1.0f;
            static_vertdata[offset+5] = 1.0f;
            static_vertdata[offset+6] = 1.0f;
        }
    }

    int sizeof_dynamic_vertdata = num_quads * 2 * 4 * sizeof(float);
    printf("sizeof_dynamic_vertdata: %d KB\n", sizeof_dynamic_vertdata/1000);
    float* dynamic_vertdata = (float*)malloc(sizeof_dynamic_vertdata);

    for (int i = 0; i < num_quads; i++) {
        auto texlookup = &TEAL_TEXCOORDS_LOOKUP;
        if (quads[i].z > 0.5f) texlookup = &DRY_TEXCOORDS_LOOKUP;
        memcpy(dynamic_vertdata + i*2*4, *texlookup, sizeof(float)*8);
    }

    int sizeof_indices = num_quads * 6 * sizeof(int);
    printf("sizeof_indices: %d KB\n", sizeof_indices/1000);
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

    GLuint static_vbo;
    glGenBuffers(1, &static_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, static_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof_static_vertdata,
        static_vertdata,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    int position_floats = 3;
    int color_floats = 4;
    int total_floats = position_floats + color_floats;
    int stride = total_floats*sizeof(float);

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

    GLuint dynamic_vbo;
    glGenBuffers(1, &dynamic_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof_dynamic_vertdata,
        dynamic_vertdata,
        GL_DYNAMIC_DRAW
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        0
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

    auto begin_time = std::chrono::high_resolution_clock::now();
    auto end_time = begin_time;
    float dt = -1.0f;

    const float CAM_SPEED = 200.0f;
    const float CAM_ZOOM_SPEED = 0.04f;

    glm::vec2 camvel = glm::vec2();

    bool wpress = false, apress = false, spress = false, dpress = false;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch (e.key.keysym.sym) {
                    case SDLK_w: wpress = true; break;
                    case SDLK_a: apress = true; break;
                    case SDLK_s: spress = true; break;
                    case SDLK_d: dpress = true; break;
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_q: {
                        camera.pos = glm::vec2();
                        camera.zoom = 1.0f;
                    } break;
                    case SDLK_p: {
                        glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
                        glBufferSubData(
                            GL_ARRAY_BUFFER,
                            0,
                            sizeof(float)*8,
                            DRY_TEXCOORDS_LOOKUP
                        );
                    } break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_w: wpress = false; break;
                    case SDLK_a: apress = false; break;
                    case SDLK_s: spress = false; break;
                    case SDLK_d: dpress = false; break;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    glm::vec2 mouse_ndc = glm::vec2(
                         (float)(e.button.x - WIDTH/2) / (float)WIDTH * 2,
                        -(float)(e.button.y - HEIGHT/2) / (float)HEIGHT * 2
                    );
                    glm::vec2 mouse_world =
                        glm::inverse(camera.proj * camera.view) *
                        glm::vec4(mouse_ndc.x, mouse_ndc.y, 0.0f, 1.0f);
                    //printf("ndc: %f, %f\n", mouse_ndc.x, mouse_ndc.y);
                    printf("world: %f, %f\n", mouse_world.x, mouse_world.y);
                    int x = mouse_world.x/size;
                    int y = mouse_world.y/size;
                    int idx = y*quads_in_row+x;
                    printf("idx: %d\n", idx);
                    quads[idx].z = 0.0f;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                float subt_from_zoom = e.wheel.preciseY * CAM_ZOOM_SPEED;
                if (camera.zoom - subt_from_zoom > 0.5f &&
                    camera.zoom - subt_from_zoom < 3.0f) {
                    camera.zoom -= subt_from_zoom;
                }
            }
        }

        float diafactor = 1.0f;
        if ((wpress || spress) && (apress || dpress)) diafactor = 0.7f;
        camvel.x = ((int)dpress - (int)apress) * CAM_SPEED * diafactor;
        camvel.y = ((int)wpress - (int)spress) * CAM_SPEED * diafactor;

        camera.pos += dt * camvel;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < num_quads; i++) {
            auto texlookup = &TEAL_TEXCOORDS_LOOKUP;
            if (quads[i].z > 0.5f) texlookup = &DRY_TEXCOORDS_LOOKUP;
            memcpy(dynamic_vertdata + i*2*4, *texlookup, sizeof(float)*8);
        }
        glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof_dynamic_vertdata,
            NULL,
            GL_DYNAMIC_DRAW
        );
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof_dynamic_vertdata,
            dynamic_vertdata,
            GL_DYNAMIC_DRAW
        );

        camera.set_proj();
        camera.set_view();
        ImGui::Text("fps: %f", 1.0f/dt);
        ImGui::Text("dt: %f", dt);
        ImGui::Text("cam_pos: (%f, %f)", camera.pos.x, camera.pos.y);
        ImGui::Text("cam_vel: (%f, %f)", camvel.x, camvel.y);
        ImGui::Text("cam_zoom: %f", camera.zoom);

        shader.use();
        shader.upload_mat4("proj", camera.proj);
        shader.upload_mat4("view", camera.view);
        glBindVertexArray(vao);
        for (int i = 0; i < 3; i++)
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
