const glm::ivec2 INVALID_TILE = glm::ivec2(-1, -1);

class TilemapBatch {
public:
    int rows;
    int halfrows;
    int num_tiles;
    glm::vec2 pos;

    // Each ivec2 specifies
    // the atlas coordinates
    glm::ivec2* tiles;
    float* vertices_data;
    int* indices_data;

    int vertices_data_len;
    int indices_data_len;

    GLuint vao, vbo, ibo;

    int VATTR_SIZE = (2+4+2+1);

    void init(int rows) {
        if (rows%2 != 0) printf("[TilemapBatch::init] rows is not even\n");
        this->rows = rows;
        this->halfrows = rows/2;
        this->num_tiles = rows*rows;

        tiles = (glm::ivec2*)Mem::alloc(num_tiles * sizeof(glm::ivec2));

        vertices_data_len = num_tiles * 4 * VATTR_SIZE;
        vertices_data = (float*)Mem::alloc(vertices_data_len * sizeof(float));

        indices_data_len = num_tiles * 6;
        indices_data = (int*)Mem::alloc(indices_data_len * sizeof(int));

        for (int i = 0; i < num_tiles; i++) {
            tiles[i] = glm::ivec2(-1, -1);
        }
    }

    void fill_vertices_data() {
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < rows; x++) {
                glm::ivec2 tilepos = glm::ivec2(x-halfrows, y-halfrows);
                glm::vec2 worldpos = glm::vec2(tilepos.x*32, tilepos.y*32);

                int idx = y*rows+x;
                glm::vec2 pos_lookup[4] = {
                    glm::vec2(worldpos.x,    worldpos.y+32),
                    glm::vec2(worldpos.x+32, worldpos.y+32),
                    glm::vec2(worldpos.x+32, worldpos.y),
                    glm::vec2(worldpos.x,    worldpos.y),
                };

                atlas.get_uvcoords(
                    glm::vec2(tiles[idx].x*32, tiles[idx].y*32),
                    glm::vec2(32, 32));

                for (int v = 0; v < 4; v++) {
                    int off = idx*4*VATTR_SIZE + v*VATTR_SIZE;

                    vertices_data[off+0] = pos_lookup[v].x;
                    vertices_data[off+1] = pos_lookup[v].y;

                    vertices_data[off+2] = 1.0f;
                    vertices_data[off+3] = 1.0f;
                    vertices_data[off+4] = 1.0f;
                    vertices_data[off+5] = 1.0f;

                    vertices_data[off+6] = uvholder[v*2];
                    vertices_data[off+7] = uvholder[v*2+1];

                    vertices_data[off+8] = 2.0f;
                }
            }
        }
    }

    void fill_indices_data() {
        for (int i = 0; i < num_tiles; i++) {
            int off = i*6;

            indices_data[off+0] = 3 + i*4;
            indices_data[off+1] = 2 + i*4;
            indices_data[off+2] = 1 + i*4;

            indices_data[off+3] = 1 + i*4;
            indices_data[off+4] = 0 + i*4;
            indices_data[off+5] = 3 + i*4;
        }
    }

    void update_render_buffers() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices_data_len * sizeof(float),
            vertices_data,
            GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices_data_len * sizeof(int),
            indices_data,
            GL_STATIC_DRAW
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

        int stride = VATTR_SIZE * sizeof(float);
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
        glDrawElements(GL_TRIANGLES, num_tiles * 6, GL_UNSIGNED_INT, 0);
    }

    glm::ivec2 get_tile(glm::ivec2 pos) {
        glm::ivec2 pos_arr = glm::ivec2(pos.x+halfrows, pos.y+halfrows);
        if (pos_arr.x >= 0 && pos_arr.x < rows && pos_arr.y >= 0 && pos_arr.y < rows) {
            return tiles[pos_arr.y*rows+pos_arr.x];
        }
        return glm::ivec2(-1, -1);
    }

    void set_tile(glm::ivec2 pos, glm::ivec2 atpos) {
        glm::ivec2 pos_arr = glm::ivec2(pos.x+halfrows, pos.y+halfrows);
        //printf("setting idx %d\n", pos_arr.y*rows+pos_arr.x);
        if (pos_arr.x >= 0 && pos_arr.x < rows && pos_arr.y >= 0 && pos_arr.y < rows) {
            tiles[pos_arr.y*rows+pos_arr.x] = atpos;
            fill_vertices_data();
            update_render_buffers();
        } else assert(0);
    }
};
