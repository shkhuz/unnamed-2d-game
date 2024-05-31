
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
