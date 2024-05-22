class Camera {
public:
    glm::mat4 proj;
    glm::mat4 view;
    glm::vec2 pos;

    Camera() {
        this->pos = glm::vec2();
    }

    void set_proj() {
        float width = 1000.0f;
        float height = (360.0f/640.0f)*width;
        printf("Setting projection to (%f, %f)\n", width, height);
        proj = glm::ortho(0.0f, width, 0.0f, height, 0.0f, 100.0f);
    }

    void set_view() {
        view = glm::lookAt(
            glm::vec3(pos.x, pos.y, 20.0f),
            glm::vec3(pos.x, pos.y, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }
};
