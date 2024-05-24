class Camera {
public:
    glm::mat4 proj;
    glm::mat4 view;
    glm::vec2 pos;
    float zoom = 1.0f;

    Camera() {
        this->pos = glm::vec2();
    }

    void set_proj() {
        float width = 1000.0f * zoom;
        float height = (360.0f/640.0f)*width;
        proj = glm::ortho(-width/2, width/2, -height/2, height/2, 0.0f, 100.0f);
    }

    void set_view() {
        view = glm::lookAt(
            glm::vec3(pos.x, pos.y, 20.0f),
            glm::vec3(pos.x, pos.y, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }
};
