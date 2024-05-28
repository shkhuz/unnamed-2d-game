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

constexpr int ROWS = 32;
constexpr float TILE_SIZE = 16.0f;
constexpr int CHUNK_ROWS = 1;

class ChunkSlot;

class TextureAtlas {
public:
    float* uv_lookup;
    int rows, width_px, tile_width_px;
    SDL_Surface* handle;

    bool load_from_file(const char* path, int rows) {
        this->rows = rows;

        SDL_Surface* img = IMG_Load(path);
        if (!img) {
            fprintf(stderr, "Unable to load texture atlas: %s\n", IMG_GetError());
            return false;
        }

        this->width_px = img->w;
        this->tile_width_px = width_px / rows;
        this->handle = img;

        uv_lookup = (float*)Mem::alloc(rows*rows * 8 * sizeof(float));
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < rows; x++) {
                int offset = (y*rows+x) * 8;

                glm::vec2 top_left =  glm::vec2((x+0)*tile_width_px+0.5f, (y+0)*tile_width_px+0.5f);
                glm::vec2 top_right = glm::vec2((x+1)*tile_width_px-0.5f, (y+0)*tile_width_px+0.5f);
                glm::vec2 bot_right = glm::vec2((x+1)*tile_width_px-0.5f, (y+1)*tile_width_px-0.5f);
                glm::vec2 bot_left =  glm::vec2((x+0)*tile_width_px+0.5f, (y+1)*tile_width_px-0.5f);

                uv_lookup[offset+0] = bot_left.x / width_px;
                uv_lookup[offset+1] = bot_left.y / width_px;

                uv_lookup[offset+2] = bot_right.x / width_px;
                uv_lookup[offset+3] = bot_right.y / width_px;

                uv_lookup[offset+4] = top_right.x / width_px;
                uv_lookup[offset+5] = top_right.y / width_px;

                uv_lookup[offset+6] = top_left.x / width_px;
                uv_lookup[offset+7] = top_left.y / width_px;
            }
        }
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
            width_px,
            width_px,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            handle->pixels
        );
        SDL_FreeSurface(handle);
    }

    float* get_uvcoords_for_tileidx(i16 idx) {
        return &uv_lookup[idx*8];
    }
};

class Chunk {
public:
    i16 data[ROWS*ROWS];
    ChunkSlot* slot;
};

Chunk chunks[CHUNK_ROWS * CHUNK_ROWS] = {
    { .data = {
        1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    }, .slot = NULL },
};

Chunk* get_chunk(glm::ivec2 pos) {
    if (pos.x >= 0 && pos.x < CHUNK_ROWS &&
        pos.y >= 0 && pos.y < CHUNK_ROWS) {
        return &chunks[pos.y*CHUNK_ROWS+pos.x];
    } else return NULL;
}

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

    static constexpr float EMPTY_TEXCOORDS_LOOKUP[8] = {
        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
    };

    glm::vec2 static_quad_data[ROWS * ROWS];
    float  static_vertex_data[STATIC_VERTEX_DATA_LEN];
    int indices_data[INDICES_DATA_LEN];
    GLuint static_vbo, ibo;


    float line_vertex_data[6 * 2];
    GLuint line_vao, line_vbo;

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
TextureAtlas texatlas;

class ChunkSlot {
public:
    float* dynamic_vertex_data;
    Chunk* current;

    glm::ivec2 pos;
    GLuint vao, dynamic_vbo;

    void allocate_client_buffers() {
        dynamic_vertex_data = (float*)Mem::alloc(
            sizeof(float) * ChunkCommonData::DYNAMIC_VERTEX_DATA_LEN
        );
    }

    void set_current_chunk(glm::ivec2 pos) {
        this->pos = pos;
        this->current = get_chunk(pos);
        if (this->current) this->current->slot = this;
    }

    void fill_dynamic_vertex_data() {
        if (current) {
            for (int q = 0; q < ROWS*ROWS; q++) {
                i16 val = current->data[q];

                const float* uvcoords = NULL;
                if (val == -1) uvcoords = ChunkCommonData::EMPTY_TEXCOORDS_LOOKUP;
                else uvcoords = texatlas.get_uvcoords_for_tileidx(val);

                int offset = q*4*ChunkCommonData::DYNAMIC_ATTR_LEN;
                memcpy(&dynamic_vertex_data[offset], uvcoords, 8*sizeof(float));
            }
        } else {
            for (int i = 0; i < ROWS*ROWS*4*2; i++) {
                dynamic_vertex_data[i] = 0.0f;
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

class Sprite {
public:
    glm::vec2 pos;
    glm::vec2 size;
    i16 texture_idx;
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
                s.pos
            };
            float* uv_lookup = texatlas.get_uvcoords_for_tileidx(s.texture_idx);

            for (int v = 0; v < 4; v++) {
                vertex_data.push_back(pos_lookup[v].x);
                vertex_data.push_back(pos_lookup[v].y);

                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);
                vertex_data.push_back(1.0f);

                vertex_data.push_back(uv_lookup[v*2]);
                vertex_data.push_back(uv_lookup[v*2+1]);
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

        int stride = (2+4+2) * sizeof(float);
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
        glBindVertexArray(0);
    }

    void render() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sprites.size() * 6, GL_UNSIGNED_INT, 0);
    }
};

i16 get_chunktile(glm::ivec2 pos, glm::ivec2 local_pos) {
    Chunk* chunk = get_chunk(pos);
    if (chunk) return chunk->data[local_pos.y*ROWS+local_pos.x];
    else return -1;
}

void set_chunktile(glm::ivec2 pos, glm::ivec2 local_pos, i16 val) {
    Chunk* chunk = get_chunk(pos);
    if (chunk) {
        chunk->data[local_pos.y*ROWS+local_pos.x] = val;
        chunk->slot->fill_dynamic_vertex_data();
        chunk->slot->update_dynamic_buffer();
    }
}

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

glm::vec2 world_pos_to_local_pos(glm::vec2 world_pos, glm::ivec2 chunk) {
    glm::vec2 local_pos = glm::vec2(
        world_pos.x - (chunk.x*32*16),
        world_pos.y - (chunk.y*32*16)
    );
    return local_pos;
}

const int CHUNKS = 9;
ChunkSlot cslots[CHUNKS];

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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    ccom = (ChunkCommonData*)Mem::alloc(sizeof(ChunkCommonData));

    if (!texatlas.load_from_file("res/atlas.png", 4)) return 1;
    Camera camera;

    ccom->init_static_quad_data();
    ccom->fill_static_vertex_data();
    ccom->fill_indices_data();
    ccom->init_render_buffers();

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cslots[i*3+j].allocate_client_buffers();
            cslots[i*3+j].set_current_chunk(glm::ivec2(j-1, i-1));
            cslots[i*3+j].fill_dynamic_vertex_data();
            cslots[i*3+j].init_render_buffers();
            cslots[i*3+j].update_dynamic_buffer();
        }
    }
    texatlas.init_for_render();

    Sprite sprite = {
        .pos = glm::vec2(100, 100),
        .size = glm::vec2(16, 16),
        .texture_idx = 4
    };
    SpriteBatch batch;
    batch.sprites.push_back(&sprite);
    batch.fill_vertex_data();
    batch.fill_indices_data();
    batch.init_render_buffers();

    ShaderProgram chunk_shader;
    if (!chunk_shader.init_from_files(
        "res/shaders/tilemap.vert",
        "res/shaders/tilemap.frag")
    ) return 1;

    ShaderProgram sprite_shader;
    if (!sprite_shader.init_from_files(
        "res/shaders/sprite.vert",
        "res/shaders/tilemap.frag")
    ) return 1;

    ShaderProgram line_shader;
    if (!line_shader.init_from_files(
        "res/shaders/dbg_line.vert",
        "res/shaders/dbg_line.frag")
    ) return 1;

    auto begin_time = std::chrono::high_resolution_clock::now();
    auto end_time = begin_time;
    float dt = -1.0f;

    bool wpress = false, apress = false, spress = false, dpress = false;
    bool lshiftpress = false;
    bool mouse_down = false;

    glm::ivec2 cam_chunk = glm::ivec2();
    glm::ivec2 last_cam_chunk = glm::ivec2();

    int tile_to_place = 0;

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
                    case SDLK_LSHIFT: lshiftpress = true; break;
                    case SDLK_ESCAPE: running = false; break;

                    case SDLK_1: tile_to_place = 0; break;
                    case SDLK_2: tile_to_place = 1; break;
                    case SDLK_3: tile_to_place = 2; break;
                    case SDLK_4: tile_to_place = 3; break;

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
                    case SDLK_LSHIFT: lshiftpress = false; break;
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
            glm::ivec2 mouse_chunk = world_pos_to_chunk(mouse_world);

            glm::vec2 tile_local_pos =
                world_pos_to_local_pos(mouse_world, mouse_chunk);
            glm::ivec2 tilexy = glm::ivec2(
                tile_local_pos.x / TILE_SIZE,
                tile_local_pos.y / TILE_SIZE
            );
            set_chunktile(
                mouse_chunk,
                tilexy,
                tile_to_place
            );
            //printf(
            //    "chunk: %d, %d  idx: %d\n",
            //    mouse_chunk.x,
            //    mouse_chunk.y,
            //    idx
            //);
        }

        sprite.pos = floor(mouse_world / 16.0f) * 16.0f;

        float diafactor = 1.0f;
        if ((wpress || spress) && (apress || dpress)) diafactor = 0.7f;
        float cam_speed_multiplier = 1.0f;
        if (lshiftpress) cam_speed_multiplier = 4.0f;
        camera.vel.x = ((int)dpress-(int)apress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;
        camera.vel.y = ((int)wpress-(int)spress) * Camera::CAM_SPEED * cam_speed_multiplier * diafactor;

        camera.pos += dt * camera.vel;

        cam_chunk = world_pos_to_chunk(camera.pos);

        if (last_cam_chunk.x != cam_chunk.x || last_cam_chunk.y != cam_chunk.y) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    cslots[i*3+j].set_current_chunk(
                        glm::ivec2((j-1)+cam_chunk.x, (i-1)+cam_chunk.y)
                    );
                    cslots[i*3+j].fill_dynamic_vertex_data();
                    cslots[i*3+j].update_dynamic_buffer();
                }
            }
        }
        batch.fill_vertex_data();
        batch.update_render_buffers();

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
        glm::vec2 cam_local_pos = world_pos_to_local_pos(camera.pos, cam_chunk);
        ImGui::Text(
            "cam_local_pos: (%f, %f)",
            cam_local_pos.x,
            cam_local_pos.y
        );

        chunk_shader.use();
        chunk_shader.upload_mat4("proj", camera.proj);
        chunk_shader.upload_mat4("view", camera.view);

        for (int i = 0; i < CHUNKS; i++) {
            chunk_shader.upload_ivec2("chunk", cslots[i].pos);
            cslots[i].render();
        }

        line_shader.use();
        line_shader.upload_mat4("proj", camera.proj);
        line_shader.upload_mat4("view", camera.view);

        for (int i = 0; i < CHUNKS; i++) {
            line_shader.upload_ivec2("chunk", cslots[i].pos);
            cslots[i].render_dbg_line();
        }

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
