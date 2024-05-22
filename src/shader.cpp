class ShaderProgram;

class Shader {
public:
    GLuint id;
    bool init_from_string(GLenum kind, const char* src);
    void attach_to(ShaderProgram* program);
    void detach_from(ShaderProgram* program);
    void destroy();
};

class ShaderProgram {
public:
    GLuint id;
    Shader vshader;
    Shader fshader;
    bool init_from_strings(const char* vs, const char* fs);
    void use();
    void upload_mat4(const char* var, glm::mat4 mat4);
};

bool Shader::init_from_string(GLenum kind, const char* src) {
    id = glCreateShader(kind);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint loglen;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &loglen);

        GLchar log[4096];
        glGetShaderInfoLog(id, loglen, NULL, log);

        const char* name;
        switch (kind) {
            case GL_VERTEX_SHADER: name = "vertex"; break;
            case GL_FRAGMENT_SHADER: name = "fragment"; break;
            default: name = "unknown"; break;
        }
        fprintf(stderr, "Compilation failure in %s shader: %s\n", name, log);
        return false;
    }
    return true;
};

void Shader::attach_to(ShaderProgram* program) {
    glAttachShader(program->id, this->id);
}

void Shader::detach_from(ShaderProgram* program) {
    glDetachShader(program->id, this->id);
}

void Shader::destroy() {
    glDeleteShader(this->id);
}

bool ShaderProgram::init_from_strings(const char* vs, const char* fs) {
    id = glCreateProgram();

    if (!vshader.init_from_string(GL_VERTEX_SHADER, vs)) return false;
    if (!fshader.init_from_string(GL_FRAGMENT_SHADER, fs)) return false;

    vshader.attach_to(this);
    fshader.attach_to(this);

    glLinkProgram(id);

    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint loglen;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen);

        GLchar log[4096];
        glGetProgramInfoLog(id, loglen, NULL, log);
        fprintf(stderr, "Shader link error: %s\n", log);
        return false;
    }

    vshader.detach_from(this);
    fshader.detach_from(this);

    return true;
}

void ShaderProgram::use() {
    glUseProgram(this->id);
}

void ShaderProgram::upload_mat4(const char* var, glm::mat4 mat4) {
    GLuint varloc = glGetUniformLocation(id, var);
    glUniformMatrix4fv(varloc, 1, GL_FALSE, glm::value_ptr(mat4));
}
