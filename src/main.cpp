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
#include <bits/stdc++.h>

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
int FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

//int WIDTH = 1920;
//int HEIGHT = 1080;
//int FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

class Random {
public:
    static float real(float range) {
        return (float)rand() / ((float)RAND_MAX/range);
    }

    // `range` is exclusive
    static int integer(int range) {
        return (int)real((int)range);
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
    int type;

    Sprite(
        glm::vec2 pos,
        glm::vec2 size,
        glm::vec2 pos_in_atlas,
        glm::vec2 size_in_atlas,
        int type
    ) {
        this->pos.x = pos.x;
        this->pos.y = pos.y;
        // TODO: make separate batch for ground tiles
        // instead of this hack.
        this->pos.z = -(pos.y - size.y);

        this->size = size;
        this->pos_in_atlas = pos_in_atlas;
        this->size_in_atlas = size_in_atlas;
        this->type = type;
    }
};

bool compare_sprite_order(Sprite s1, Sprite s2) {
    return s1.pos.z < s2.pos.z;
}

class SpriteBatch {
public:
    std::vector<Sprite> sprites;

    std::vector<float> vertex_data;
    std::vector<int> indices_data;

    GLuint vao, vbo, ibo;

    void fill_vertex_data() {
        vertex_data.clear();
        vertex_data.reserve(1000 * 4 * (2+4+2+1));

        std::sort(sprites.begin(), sprites.end(), compare_sprite_order);
        for (usize i = 0; i < sprites.size(); i++) {
            Sprite s = sprites[i];
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

                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);

                vertex_data.push_back(uvholder[v*2]);
                vertex_data.push_back(uvholder[v*2+1]);

                vertex_data.push_back((float)s.type);
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
        glBindVertexArray(vao);
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
        glEnableVertexAttribArray(3);

        int stride = (2+4+2+1) * sizeof(float);
        glVertexAttribPointer(
            0,
            2,
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
            (void*)((2) * sizeof(float))
        );
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void*)((2+4) * sizeof(float))
        );
        glVertexAttribPointer(
            3,
            1,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void*)((2+4+2) * sizeof(float))
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

bool lie_in_rect(glm::vec2 p, glm::vec2 rect_pos, glm::vec2 rect_size) {
    if (p.x >= rect_pos.x && p.y > rect_pos.y &&
        p.x <= rect_pos.x+rect_size.x && p.y <= rect_pos.y+rect_size.y) {
        return true;
    }
    return false;
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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_dbg_callback, NULL);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    if (!atlas.load_from_file("res/atlas.png")) return 1;
    atlas.init_for_render();

    Camera camera;

    SpriteBatch batch;
    glm::vec2 grass_pos = glm::vec2(0, 0);
    glm::vec2 grass_size = glm::vec2(32, 23);
    const glm::vec4 tree1 = glm::vec4(0, 23, 32, 64);
    const glm::vec4 tree2 = glm::vec4(0, 23+64, 32, 70);

    for (int y = -8; y < 8; y++) {
        for (int x = -8; x < 8; x++) {
            batch.sprites.push_back(Sprite(
                glm::vec2(x*grass_size.x, y*grass_size.y),
                grass_size,
                grass_pos,
                grass_size,
                0
            ));
        }
    }

    int num_trees = 21;//Random::integer(20);
    printf("num_trees: %d\n", num_trees);
    for (int i = 0; i < num_trees; i++) {
        int x = Random::integer(16) - 8;
        int y = Random::integer(16) - 8;
        const glm::vec4& tree = Random::real(1) > 0.7f ? tree2 : tree1;
        batch.sprites.push_back(Sprite(
            glm::vec2(x*grass_size.x, y*grass_size.y),
            glm::vec2(tree.z, tree.w),
            glm::vec2(tree.x, tree.y),
            glm::vec2(tree.z, tree.w),
            1
        ));
    }

    batch.fill_vertex_data();
    batch.fill_indices_data();
    batch.init_render_buffers();

    /* for (usize i = 0; i < batch.vertex_data.size(); i++) { */
    /*     printf("%9.3f ", batch.vertex_data[i]); */
    /*     if ((i+1) % 9 == 0) printf("\n"); */
    /*     if ((i+1) % 36 == 0) printf("\n"); */
    /* } */

    ShaderProgram sprite_shader;
    if (!sprite_shader.init_from_files(
        "res/shaders/sprite.v.glsl",
        "res/shaders/sprite.f.glsl")
    ) return 1;

    auto start_time = std::chrono::high_resolution_clock::now();
    auto begin_time = start_time;
    auto end_time = begin_time;
    float dt = -1.0f;
    u64 frames = 0;

    bool wpress = false, apress = false, spress = false, dpress = false;
    bool lmouse_down = false, just_lmouse_down = false;
    bool rmouse_down = false, just_rmouse_down = false;
    int tree_type = 0;

    printf("Mem::alloc(): allocated %lu KB\n", Mem::allocated/1000);
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                continue;
            }
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
                    lmouse_down = true;
                    just_lmouse_down = true;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    rmouse_down = true;
                    just_rmouse_down = true;
                }

            } else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    lmouse_down = false;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    rmouse_down = false;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                float subt_from_zoom = e.wheel.preciseY * Camera::CAM_ZOOM_SPEED;
                //if (camera.zoom - subt_from_zoom >= 0.5f &&
                //    camera.zoom - subt_from_zoom < 1.0f) {
                    camera.zoom -= subt_from_zoom;
                //}
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    WIDTH = e.window.data1;
                    HEIGHT = e.window.data2;
                    glViewport(0, 0, WIDTH, HEIGHT);
                }
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

        if (just_lmouse_down) {
            glm::vec2 tilepos = floor(mouse_world / grass_size) * grass_size;
            const glm::vec4& tree = tree_type == 1 ? tree1 : tree2;
            batch.sprites.push_back(Sprite {
                tilepos,
                glm::vec2(tree.z, tree.w),
                glm::vec2(tree.x, tree.y),
                glm::vec2(tree.z, tree.w),
                1
            });
        }

        if (just_rmouse_down) {
            for (usize i = batch.sprites.size(); i --> 0;) {
                if (batch.sprites[i].type == 1 &&
                    lie_in_rect(mouse_world, batch.sprites[i].pos, batch.sprites[i].size)) {
                    batch.sprites.erase(batch.sprites.begin()+i);
                    break;
                }
            }
        }

        float diafactor = 1.0f;
        if ((wpress || spress) && (apress || dpress)) diafactor = 0.7f;
        float cam_speed_multiplier = 1.0f;
        camera.vel.x = ((int)dpress-(int)apress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;
        camera.vel.y = ((int)wpress-(int)spress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;

        camera.pos += dt * camera.vel;

        batch.fill_vertex_data();
        batch.fill_indices_data();
        batch.update_render_buffers();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT);

        camera.set_proj();
        camera.set_view();
        ImGui::Text("fps: %f", 1.0f/dt);
        double elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time)
            .count() / 1e9;
        ImGui::Text("avg_fps: %lf", ((double)frames)/elapsed);
        ImGui::Text("dt: %f", dt);
        ImGui::Text("cam_pos: (%f, %f)", camera.pos.x, camera.pos.y);
        ImGui::Text("cam_vel: (%f, %f)", camera.vel.x, camera.vel.y);
        ImGui::Text("cam_zoom: %f", camera.zoom);
        ImGui::Text("mouse_world: (%f, %f)", mouse_world.x, mouse_world.y);
        ImGui::SliderInt("tree_type", &tree_type, 0, 1);

        sprite_shader.use();
        sprite_shader.upload_mat4("proj", camera.proj);
        sprite_shader.upload_mat4("view", camera.view);
        sprite_shader.upload_float("time", (float)elapsed);
        batch.render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        end_time = std::chrono::high_resolution_clock::now();
        dt =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time)
            .count() / 1e9;
        begin_time = end_time;
        frames++;
        just_lmouse_down = false;
        just_rmouse_down = false;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    IMG_Quit();
    SDL_Quit();
}
