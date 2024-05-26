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

//int WIDTH = 1280;
//int HEIGHT = 720;
//int FLAGS = SDL_WINDOW_OPENGL;

int WIDTH = 1920;
int HEIGHT = 1080;
int FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

class Random {
public:
    static float real() {
        return (float)rand() / ((float)RAND_MAX/1.0f);
    }
};

class Math {
public:
    static int smart_floor(float x) {
        if (x >= 0.0f) return (int)x;
        else return (int)(x-1.0f);
    }
};

class Mem {
public:
    inline static usize allocated;
    static void* alloc(usize bytes) {
        allocated += bytes;
        return malloc(bytes);
    }
};

constexpr int ROWS = 32;
constexpr float TILE_SIZE = 16.0f;

class ChunkCommonData {
public:
    static const int POSITION_ATTR_LEN = 2;
    static const int COLOR_ATTR_LEN = 4;
    static const int TEXCOORS_ATTR_LEN = 2;

    static const int STATIC_ATTR_LEN = POSITION_ATTR_LEN + COLOR_ATTR_LEN;
    static const int DYNAMIC_ATTR_LEN = TEXCOORS_ATTR_LEN;

    static const int STATIC_VERTEX_DATA_LEN = ROWS*ROWS * 4 * STATIC_ATTR_LEN;
    static const int DYNAMIC_VERTEX_DATA_LEN = ROWS*ROWS * 4 * DYNAMIC_ATTR_LEN;

    static const int INDICES_DATA_LEN = ROWS*ROWS * 6;

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

    float  static_vertex_data[STATIC_VERTEX_DATA_LEN];
    int indices_data[INDICES_DATA_LEN];
    GLuint static_vbo, ibo;
    GLuint line_vao, line_vbo;

    glm::vec2 static_quad_data[ROWS * ROWS];

    float line_vertex_data[6 * 2];

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

    void fill_static_vertex_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            for (int v = 0; v < 4; v++) {
                int offset = q*4*STATIC_ATTR_LEN + v*STATIC_ATTR_LEN;
                static_vertex_data[offset] =
                    static_quad_data[q].x + VERTEX_OFFSET_LOOKUP[v].x;
                static_vertex_data[offset+1] =
                    static_quad_data[q].y + VERTEX_OFFSET_LOOKUP[v].y;

                static_vertex_data[offset+2] = 1.0f;
                static_vertex_data[offset+3] = 1.0f;
                static_vertex_data[offset+4] = 1.0f;
                static_vertex_data[offset+5] = 1.0f;

            }
        }

        line_vertex_data[0] = 0.0f;
        line_vertex_data[1] = 0.0f;

        line_vertex_data[2] = 32*16.0f;
        line_vertex_data[3] = 0.0f;

        line_vertex_data[4] = 32*16.0f;
        line_vertex_data[5] = 32*16.0f;

        line_vertex_data[6] = 32*16.0f;
        line_vertex_data[7] = 32*16.0f;

        line_vertex_data[8] = 0.0f;
        line_vertex_data[9] = 32*16.0f;

        line_vertex_data[10] = 0.0f;
        line_vertex_data[11] = 0.0f;
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

    void init_render_buffers() {
        glGenBuffers(1, &static_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, static_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            STATIC_VERTEX_DATA_LEN*sizeof(float),
            static_vertex_data,
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            INDICES_DATA_LEN*sizeof(int),
            indices_data,
            GL_STATIC_DRAW
        );

        glGenVertexArrays(1, &line_vao);
        glBindVertexArray(line_vao);
        glGenBuffers(1, &line_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            12*sizeof(float),
            line_vertex_data,
            GL_STATIC_DRAW
        );
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
};

ChunkCommonData* ccom = NULL;

class ChunkSlot {
public:
    float* dynamic_vertex_data;
    float* dynamic_quad_data;

    glm::ivec2 pos;
    GLuint vao, dynamic_vbo;

    void allocate_client_buffers() {
        dynamic_vertex_data = (float*)Mem::alloc(
            sizeof(float) * ChunkCommonData::DYNAMIC_VERTEX_DATA_LEN
        );
        dynamic_quad_data = (float*)Mem::alloc(sizeof(float) * ROWS * ROWS);
    }

    void load_dynamic_quad_data(glm::ivec2 pos) {
        this->pos = pos;
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < ROWS; j++) {
                dynamic_quad_data[j*ROWS+i] = Random::real();
            }
        }
    }

    void fill_dynamic_vertex_data() {
        for (int q = 0; q < ROWS*ROWS; q++) {
            auto tile = &ChunkCommonData::TEAL_TEXCOORDS_LOOKUP;
            if (dynamic_quad_data[q] > 0.5f)
                tile = &ChunkCommonData::DRY_TEXCOORDS_LOOKUP;
            for (int v = 0; v < 4; v++) {
                int offset = q*4*ChunkCommonData::DYNAMIC_ATTR_LEN +
                    v*ChunkCommonData::DYNAMIC_ATTR_LEN;
                dynamic_vertex_data[offset]   = (*tile)[v*2];
                dynamic_vertex_data[offset+1] = (*tile)[v*2+1];
            }
        }
    }

    void update_dynamic_buffer() {
        glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            ChunkCommonData::DYNAMIC_VERTEX_DATA_LEN*sizeof(float),
            NULL,
            GL_DYNAMIC_DRAW
        );
        glBufferData(
            GL_ARRAY_BUFFER,
            ChunkCommonData::DYNAMIC_VERTEX_DATA_LEN*sizeof(float),
            dynamic_vertex_data,
            GL_DYNAMIC_DRAW
        );
    }

    void init_render_buffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &dynamic_vbo);
        update_dynamic_buffer();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ccom->ibo);

        glBindBuffer(GL_ARRAY_BUFFER, ccom->static_vbo);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            0,
            ChunkCommonData::POSITION_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            ChunkCommonData::STATIC_ATTR_LEN*sizeof(float),
            (void*)(0)
        );
        glVertexAttribPointer(
            1,
            ChunkCommonData::COLOR_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            ChunkCommonData::STATIC_ATTR_LEN*sizeof(float),
            (void*)(ChunkCommonData::POSITION_ATTR_LEN*sizeof(float))
        );

        glBindBuffer(GL_ARRAY_BUFFER, dynamic_vbo);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            ChunkCommonData::TEXCOORS_ATTR_LEN,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)(0)
        );
    }

    void render() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, ROWS*ROWS * 6, GL_UNSIGNED_INT, 0);
    }

    void render_dbg_line() {
        glBindVertexArray(ccom->line_vao);
        glDrawArrays(GL_LINES, 0, 6);
    }
};

glm::ivec2 world_pos_to_chunk(glm::vec2 world_pos) {
    glm::vec2 chunkf = glm::vec2(
        world_pos.x / (ROWS*TILE_SIZE),
        world_pos.y / (ROWS*TILE_SIZE)
    );
    glm::ivec2 chunki = glm::ivec2(
        Math::smart_floor(chunkf.x),
        Math::smart_floor(chunkf.y)
    );
    return chunki;
}

glm::vec2 world_pos_to_relchunk_pos(glm::vec2 world_pos, glm::ivec2 chunk) {
    glm::vec2 relchunk_pos = glm::vec2(
        world_pos.x - (chunk.x*32*16),
        world_pos.y - (chunk.y*32*16)
    );
    return relchunk_pos;
}

const int CHUNKS = 9;
ChunkSlot cslots[CHUNKS];

ChunkSlot* get_chunk(glm::ivec2 chunk) {
    for (int i = 0; i < 9; i++) {
        if (cslots[i].pos == chunk) return &cslots[i];
    }
    return NULL;
}

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

    ccom = (ChunkCommonData*)Mem::alloc(sizeof(ChunkCommonData));

    Camera camera;

    ccom->init_static_quad_data();
    ccom->fill_static_vertex_data();
    ccom->fill_indices_data();
    ccom->init_render_buffers();

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cslots[i*3+j].allocate_client_buffers();
            cslots[i*3+j].load_dynamic_quad_data(glm::ivec2(j-1, i-1));
            cslots[i*3+j].fill_dynamic_vertex_data();
            cslots[i*3+j].update_dynamic_buffer();
            cslots[i*3+j].init_render_buffers();
        }
    }

    ShaderProgram shader;
    if (!shader.init_from_files(
        "res/shaders/tilemap.vert",
        "res/shaders/tilemap.frag")
    ) return 1;

    ShaderProgram line_shader;
    if (!line_shader.init_from_files(
        "res/shaders/dbg_line.vert",
        "res/shaders/dbg_line.frag")
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

    glm::ivec2 cam_chunk = glm::ivec2();
    glm::ivec2 last_cam_chunk = glm::ivec2();

    printf("allocated %lu KB\n", Mem::allocated/1000);
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
                    glm::ivec2 mouse_chunk = world_pos_to_chunk(mouse_world);
                    ChunkSlot* cslot = get_chunk(mouse_chunk);
                    glm::vec2 tilepos_relchunk =
                        world_pos_to_relchunk_pos(mouse_world, mouse_chunk);
                    glm::ivec2 tilexy = glm::ivec2(
                        tilepos_relchunk.x / TILE_SIZE,
                        tilepos_relchunk.y / TILE_SIZE
                    );
                    int idx = tilexy.y*ROWS+tilexy.x;
                    printf(
                        "chunk: %d, %d  idx: %d  tile: %f\n",
                        mouse_chunk.x,
                        mouse_chunk.y,
                        idx,
                        cslot->dynamic_quad_data[idx]
                    );
                    cslot->dynamic_quad_data[idx] = 0.0f;
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

        cam_chunk = world_pos_to_chunk(camera.pos);

        if (last_cam_chunk.x != cam_chunk.x || last_cam_chunk.y != cam_chunk.y) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    cslots[i*3+j].load_dynamic_quad_data(
                        glm::ivec2((j-1)+cam_chunk.x, (i-1)+cam_chunk.y)
                    );
                }
            }
        }
        for (int i = 0; i < 9; i++) {
            cslots[i].fill_dynamic_vertex_data();
            cslots[i].update_dynamic_buffer();
        }

        last_cam_chunk = cam_chunk;

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
        ImGui::Text("cam_chunk: (%d, %d)", cam_chunk.x, cam_chunk.y);
        glm::vec2 cam_relchunk_pos = world_pos_to_relchunk_pos(camera.pos, cam_chunk);
        ImGui::Text(
            "cam_relchunk_pos: (%f, %f)",
            cam_relchunk_pos.x,
            cam_relchunk_pos.y
        );

        shader.use();
        shader.upload_mat4("proj", camera.proj);
        shader.upload_mat4("view", camera.view);

        for (int i = 0; i < CHUNKS; i++) {
            shader.upload_ivec2("chunk", cslots[i].pos);
            cslots[i].render();
        }

        line_shader.use();
        line_shader.upload_mat4("proj", camera.proj);
        line_shader.upload_mat4("view", camera.view);

        for (int i = 0; i < CHUNKS; i++) {
            line_shader.upload_ivec2("chunk", cslots[i].pos);
            cslots[i].render_dbg_line();
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
