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
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef ssize_t isize;

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
        printf("Mem::alloc(): allocating %lu bytes\n", bytes);
        allocated += bytes;
        return malloc(bytes);
    }
};

static float uvholder[8];

class TextureAtlas {
public:
    int width, height;
    SDL_Surface* handle;

    bool load_from_file(const char* path) {
        SDL_Surface* img = IMG_Load(path);
        if (!img) {
            fprintf(stderr, "Unable to load texture atlas: %s\n", IMG_GetError());
            return false;
        }

        this->width = img->w;
        this->height = img->h;
        this->handle = img;
        return true;
    }

    void init_for_render() {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            handle->pixels
        );
        SDL_FreeSurface(handle);
    }

    void get_uvcoords(glm::vec2 pos, glm::vec2 size) {
        //pos.x += 0.5f;
        //pos.y += 0.5f;
        //size.x -= 0.5f;
        //size.y -= 0.5f;

        uvholder[0] = pos.x / width;
        uvholder[1] = pos.y / height;

        uvholder[2] = (pos.x+size.x) / width;
        uvholder[3] = pos.y / height;

        uvholder[4] = (pos.x+size.x) / width;
        uvholder[5] = (pos.y+size.y) / height;

        uvholder[6] = pos.x / width;
        uvholder[7] = (pos.y+size.y) / height;
    }
};

TextureAtlas atlas;

class Sprite {
public:
    glm::vec3 pos;
    glm::vec2 size;
    glm::vec2 pos_in_atlas;
    glm::vec2 size_in_atlas;
};

class SpriteBatch {
public:
    std::vector<Sprite*> sprites;

    std::vector<float> vertex_data;
    std::vector<int> indices_data;

    GLuint vao, vbo, ibo;

    void fill_vertex_data() {
        vertex_data.clear();
        vertex_data.reserve(1000 * 4 * (2+4+2));

        for (usize i = 0; i < sprites.size(); i++) {
            Sprite s = *sprites[i];
            glm::vec2 pos_lookup[4] = {
                glm::vec2(s.pos.x,          s.pos.y+s.size.y),
                glm::vec2(s.pos.x+s.size.x, s.pos.y+s.size.y),
                glm::vec2(s.pos.x+s.size.x, s.pos.y),
                glm::vec2(s.pos.x,          s.pos.y),
            };

            atlas.get_uvcoords(s.pos_in_atlas, s.size_in_atlas);

            for (int v = 0; v < 4; v++) {
                vertex_data.push_back(pos_lookup[v].x);
                vertex_data.push_back(pos_lookup[v].y);
                vertex_data.push_back(s.pos.z);

                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);

                vertex_data.push_back(uvholder[v*2]);
                vertex_data.push_back(uvholder[v*2+1]);
            }
        }
    }

    void fill_indices_data() {
        indices_data.clear();
        indices_data.reserve(1000 * 6);

        for (usize i = 0; i < sprites.size(); i++) {
            indices_data.push_back(3 + i*4);
            indices_data.push_back(2 + i*4);
            indices_data.push_back(1 + i*4);

            indices_data.push_back(1 + i*4);
            indices_data.push_back(0 + i*4);
            indices_data.push_back(3 + i*4);
        }
    }

    void update_render_buffers() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertex_data.size() * sizeof(float),
            vertex_data.data(),
            GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices_data.size() * sizeof(int),
            indices_data.data(),
            GL_DYNAMIC_DRAW
        );
    }

    void init_render_buffers() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        update_render_buffers();

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        int stride = (3+4+2) * sizeof(float);
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
            (void*)((3) * sizeof(float))
        );
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void*)((3+4) * sizeof(float))
        );
        glBindVertexArray(0);
    }

    void render() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sprites.size() * 6, GL_UNSIGNED_INT, 0);
    }
};

void opengl_dbg_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    printf("OPENGL: %s\n", message);
}

int main() {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

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
    //ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(window, glctx);
    ImGui_ImplOpenGL3_Init();

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_dbg_callback, NULL);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    if (!atlas.load_from_file("res/atlas.png")) return 1;
    atlas.init_for_render();

    Camera camera;

    Sprite sprite = {
        .pos = glm::vec3(0, 0, 20),
        .size = glm::vec2(32, 44),
        .pos_in_atlas = glm::vec2(0, 0),
        .size_in_atlas = glm::vec2(32, 44),
    };
    Sprite sprite2 = {
        .pos = glm::vec3(32, 0, 20),
        .size = glm::vec2(32, 44),
        .pos_in_atlas = glm::vec2(0, 0),
        .size_in_atlas = glm::vec2(32, 44),
    };
    Sprite sprite3 = {
        .pos = glm::vec3(0, 22, 19),
        .size = glm::vec2(32, 44),
        .pos_in_atlas = glm::vec2(0, 0),
        .size_in_atlas = glm::vec2(32, 44),
    };
    Sprite sprite4 = {
        .pos = glm::vec3(32, 22, 19),
        .size = glm::vec2(32, 44),
        .pos_in_atlas = glm::vec2(0, 0),
        .size_in_atlas = glm::vec2(32, 44),
    };

    SpriteBatch batch;
    batch.sprites.push_back(&sprite);
    batch.sprites.push_back(&sprite2);
    batch.sprites.push_back(&sprite3);
    batch.sprites.push_back(&sprite4);
    batch.fill_vertex_data();
    batch.fill_indices_data();
    batch.init_render_buffers();

    for (usize i = 0; i < batch.vertex_data.size(); i++) {
        printf("%7.3f ", batch.vertex_data[i]);
        if ((i+1) % 9 == 0) printf("\n");
        if ((i+1) % 36 == 0) printf("\n");
    }

    ShaderProgram sprite_shader;
    if (!sprite_shader.init_from_files(
        "res/shaders/sprite.vert",
        "res/shaders/sprite.frag")
    ) return 1;

    auto begin_time = std::chrono::high_resolution_clock::now();
    auto end_time = begin_time;
    float dt = -1.0f;

    bool wpress = false, apress = false, spress = false, dpress = false;
    bool mouse_down = false;

    printf("Mem::alloc(): allocated %lu KB\n", Mem::allocated/1000);
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
                    mouse_down = true;
                }

            } else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    mouse_down = false;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                float subt_from_zoom = e.wheel.preciseY * Camera::CAM_ZOOM_SPEED;
                //if (camera.zoom - subt_from_zoom >= 0.5f &&
                //    camera.zoom - subt_from_zoom < 1.0f) {
                    camera.zoom -= subt_from_zoom;
                //}
            }
        }

        int x, y;
        SDL_GetMouseState(&x, &y);
        glm::vec2 mouse_ndc = glm::vec2(
             (float)(x - WIDTH/2) / (float)WIDTH * 2,
            -(float)(y - HEIGHT/2) / (float)HEIGHT * 2
        );
        glm::vec2 mouse_world =
            glm::inverse(camera.proj * camera.view) *
            glm::vec4(mouse_ndc.x, mouse_ndc.y, 0.0f, 1.0f);

        if (mouse_down) {
        }

        //sprite.pos = floor(mouse_world / 16.0f) * 16.0f;
        //sprite.pos = mouse_world;

        float diafactor = 1.0f;
        if ((wpress || spress) && (apress || dpress)) diafactor = 0.7f;
        float cam_speed_multiplier = 1.0f;
        camera.vel.x = ((int)dpress-(int)apress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;
        camera.vel.y = ((int)wpress-(int)spress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;

        camera.pos += dt * camera.vel;

        batch.fill_vertex_data();
        batch.update_render_buffers();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.set_proj();
        camera.set_view();
        ImGui::Text("fps: %f", 1.0f/dt);
        ImGui::Text("dt: %f", dt);
        ImGui::Text("cam_pos: (%f, %f)", camera.pos.x, camera.pos.y);
        ImGui::Text("cam_vel: (%f, %f)", camera.vel.x, camera.vel.y);
        ImGui::Text("cam_zoom: %f", camera.zoom);

        sprite_shader.use();
        sprite_shader.upload_mat4("proj", camera.proj);
        sprite_shader.upload_mat4("view", camera.view);
        batch.render();

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
