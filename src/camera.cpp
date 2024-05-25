class Camera {
public:
    static constexpr float CAM_SPEED = 200.0f;
    static constexpr float CAM_ZOOM_SPEED = 0.04f;

    glm::mat4 proj;
    glm::mat4 view;

    float zoom = 1.0f;
    glm::vec2 pos = glm::vec2();
    glm::vec2 vel = glm::vec2();

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
