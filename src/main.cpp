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

//int WIDTH = 640;
//int HEIGHT = 360;
//int FLAGS = SDL_WINDOW_OPENGL;

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

class Math {
public:
    static int smart_floor(float x) {
        if (x >= 0.0f) return (int)x;
        else return (int)(x-1.0f);
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

    static constexpr float TEAL_TEXCOORDS_LOOKUP[] = {
        0.0f, 0.0f,
        0.5f, 0.0f,
        0.5f, 1.0f,
        0.0f, 1.0f,
    };

    static constexpr float DRY_TEXCOORDS_LOOKUP[] = {
        0.5f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.5f, 1.0f,
    };

    static const int POSITION_ATTR_LEN = 2;
    static const int COLOR_ATTR_LEN = 4;
    static const int TEXCOORS_ATTR_LEN = 2;
    static const int STATIC_ATTR_LEN = POSITION_ATTR_LEN + COLOR_ATTR_LEN;
    static const int DYNAMIC_ATTR_LEN = TEXCOORS_ATTR_LEN;

    static const int STATIC_VERTEX_DATA_LEN = ROWS*ROWS * 4 * STATIC_ATTR_LEN;
    static const int DYNAMIC_VERTEX_DATA_LEN = ROWS*ROWS * 4 * DYNAMIC_ATTR_LEN;
    static const int INDICES_DATA_LEN = ROWS*ROWS * 6;

    float  static_vertex_data[STATIC_VERTEX_DATA_LEN];
    float dynamic_vertex_data[DYNAMIC_VERTEX_DATA_LEN];
    int indices_data[INDICES_DATA_LEN];

    GLuint vao, static_vbo, dynamic_vbo, ibo;

    glm::vec2 static_quad_data[ROWS * ROWS];
    float dynamic_quad_data[ROWS * ROWS];
    int chunkx = 0, chunky = 0;

    void init_static_quad_data() {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < ROWS; j++) {
                static_quad_data[j*ROWS+i] = glm::vec2(
                    i*TILE_SIZE,
                    j*TILE_SIZE
                );
            }
        }
    }

    void load_dynamic_quad_data(int chunkx, int chunky) {
        //printf("loading dynamic quad data for %d, %d\n", chunkx, chunky);
        this->chunkx = chunkx;
        this->chunky = chunky;
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < ROWS; j++) {
                dynamic_quad_data[j*ROWS+i] = Random::real();
            }
        }
    }

    void fill_static_vertex_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            for (int v = 0; v < 4; v++) {
                int offset = q*4*STATIC_ATTR_LEN + v*STATIC_ATTR_LEN;
                static_vertex_data[offset]   = static_quad_data[q].x + VERTEX_OFFSET_LOOKUP[v].x;
                static_vertex_data[offset+1] = static_quad_data[q].y + VERTEX_OFFSET_LOOKUP[v].y;

                static_vertex_data[offset+2] = 1.0f;
                static_vertex_data[offset+3] = 1.0f;
                static_vertex_data[offset+4] = 1.0f;
                static_vertex_data[offset+5] = 1.0f;

            }
        }
    }

    void fill_dynamic_vertex_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            auto tile = &TEAL_TEXCOORDS_LOOKUP;
            if (dynamic_quad_data[q] > 0.5f) tile = &DRY_TEXCOORDS_LOOKUP;
            for (int v = 0; v < 4; v++) {
                int offset = q*4*DYNAMIC_ATTR_LEN + v*DYNAMIC_ATTR_LEN;
                dynamic_vertex_data[offset]   = (*tile)[v*2];
                dynamic_vertex_data[offset+1] = (*tile)[v*2+1];
            }
        }
    }

    void fill_indices_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            int offset = q * 6;
            indices_data[offset]   = 3 + q*4;
            indices_data[offset+1] = 2 + q*4;
            indices_data[offset+2] = 1 + q*4;

            indices_data[offset+3] = 1 + q*4;
            indices_data[offset+4] = 0 + q*4;
            indices_data[offset+5] = 3 + q*4;
        }
    }

    void update_dynamic_buffer() {
        glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            DYNAMIC_VERTEX_DATA_LEN*sizeof(float),
            NULL,
            GL_DYNAMIC_DRAW
        );
        glBufferData(
            GL_ARRAY_BUFFER,
            DYNAMIC_VERTEX_DATA_LEN*sizeof(float),
            dynamic_vertex_data,
            GL_DYNAMIC_DRAW
        );
    }

    void init_render_buffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &static_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, static_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            STATIC_VERTEX_DATA_LEN*sizeof(float),
            static_vertex_data,
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            0,
            POSITION_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            STATIC_ATTR_LEN*sizeof(float),
            (void*)(0)
        );
        glVertexAttribPointer(
            1,
            COLOR_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            STATIC_ATTR_LEN*sizeof(float),
            (void*)(POSITION_ATTR_LEN*sizeof(float))
        );

        glGenBuffers(1, &dynamic_vbo);
        update_dynamic_buffer();

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            TEXCOORS_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)(0)
        );

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            INDICES_DATA_LEN*sizeof(int),
            indices_data,
            GL_STATIC_DRAW
        );

        glBindVertexArray(0);
    }

    void render() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, ROWS*ROWS * 6, GL_UNSIGNED_INT, 0);
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

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cslots[i*3+j].init_static_quad_data();
            cslots[i*3+j].load_dynamic_quad_data(j-1, i-1);
            cslots[i*3+j].fill_static_vertex_data();
            cslots[i*3+j].fill_dynamic_vertex_data();
            cslots[i*3+j].fill_indices_data();
            cslots[i*3+j].init_render_buffers();
        }
    }

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

    bool wpress = false, apress = false, spress = false, dpress = false;

    int last_chunkx = 0, last_chunky = 0;

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
                    //int x = mouse_world.x/size;
                    //int y = mouse_world.y/size;
                    //int idx = y*quads_in_row+x;
                    //printf("idx: %d\n", idx);
                    //quads[idx].z = 0.0f;
                }

            } else if (e.type == SDL_MOUSEWHEEL) {
                float subt_from_zoom = e.wheel.preciseY * Camera::CAM_ZOOM_SPEED;
                if (camera.zoom - subt_from_zoom >= 0.5f &&
                    camera.zoom - subt_from_zoom < 1.0f) {
                    camera.zoom -= subt_from_zoom;
                }
            }
        }

        float diafactor = 1.0f;
        if ((wpress || spress) && (apress || dpress)) diafactor = 0.7f;
        camera.vel.x = ((int)dpress - (int)apress) * Camera::CAM_SPEED * diafactor;
        camera.vel.y = ((int)wpress - (int)spress) * Camera::CAM_SPEED * diafactor;

        camera.pos += dt * camera.vel;

        glm::vec2 cam_chunkf = glm::vec2(
            camera.pos.x / (ChunkSlot::ROWS*ChunkSlot::TILE_SIZE),
            camera.pos.y / (ChunkSlot::ROWS*ChunkSlot::TILE_SIZE)
        );
        int cam_chunkx = Math::smart_floor(cam_chunkf.x);
        int cam_chunky = Math::smart_floor(cam_chunkf.y);

        if (last_chunkx != cam_chunkx || last_chunky != cam_chunky) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    cslots[i*3+j].load_dynamic_quad_data((j-1)+cam_chunkx, (i-1)+cam_chunky);
                    cslots[i*3+j].fill_dynamic_vertex_data();
                    cslots[i*3+j].update_dynamic_buffer();
                }
            }
        }

        last_chunkx = cam_chunkx;
        last_chunky = cam_chunky;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera.set_proj();
        camera.set_view();
        ImGui::Text("fps: %f", 1.0f/dt);
        ImGui::Text("dt: %f", dt);
        ImGui::Text("cam_pos: (%f, %f)", camera.pos.x, camera.pos.y);
        ImGui::Text("cam_vel: (%f, %f)", camera.vel.x, camera.vel.y);
        ImGui::Text("cam_zoom: %f", camera.zoom);
        ImGui::Text("cam_chunkf: (%f, %f)", cam_chunkf.x, cam_chunkf.y);
        ImGui::Text("cam_chunk: (%d, %d)", cam_chunkx, cam_chunky);

        shader.use();
        shader.upload_mat4("proj", camera.proj);
        shader.upload_mat4("view", camera.view);

        for (int i = 0; i < CHUNKS; i++) {
            shader.upload_int2("chunk", cslots[i].chunkx, cslots[i].chunky);
            cslots[i].render();
        }

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
