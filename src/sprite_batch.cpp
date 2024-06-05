class SpriteBatch {
public:
    std::vector<Sprite> sprites;
    std::vector<float> vertices_data;
    std::vector<int> indices_data;

    GLuint vao, vbo, ibo;

    void fill_vertices_data() {
        vertices_data.clear();
        vertices_data.reserve(1000 * 4 * (2+4+2+1));

        //std::sort(sprites.begin(), sprites.end(), compare_sprite_order);

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
                vertices_data.push_back(pos_lookup[v].x);
                vertices_data.push_back(pos_lookup[v].y);

                vertices_data.push_back(1.0f);
                vertices_data.push_back(1.0f);
                vertices_data.push_back(1.0f);
                vertices_data.push_back(1.0f);

                vertices_data.push_back(uvholder[v*2]);
                vertices_data.push_back(uvholder[v*2+1]);

                vertices_data.push_back((float)s.type);
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
            vertices_data.size() * sizeof(float),
            vertices_data.data(),
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
