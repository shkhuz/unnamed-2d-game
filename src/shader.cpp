class ShaderProgram;

class Shader {
public:
    GLuint id;
    bool init_from_string(GLenum kind, const char* src);
    bool init_from_file(GLenum kind, const char* path);
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
    bool init_from_files(const char* vspath, const char* fspath);
    bool lateinit();
    void use();
    void upload_mat4(const char* var, glm::mat4 mat4);
    void upload_float(const char* var, float f);
    void upload_ivec2(const char* var, glm::ivec2 ivec2);
};

static const char* shader_kind_to_string(GLenum kind) {
    const char* name;
    switch (kind) {
        case GL_VERTEX_SHADER: name = "vertex"; break;
        case GL_FRAGMENT_SHADER: name = "fragment"; break;
        default: name = "unknown"; break;
    }
    return name;
}

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

        fprintf(
            stderr,
            "Compilation failure in %s shader: %s\n",
            shader_kind_to_string(kind),
            log
        );
        return false;
    }
    return true;
};

bool Shader::init_from_file(GLenum kind, const char* path) {
    FileOrError src = read_file(path);
    if (src.status != FILEOP_SUCCESS) {
        fprintf(stderr, "Error reading %s shader in path %s\n", shader_kind_to_string(kind), path);
        return false;
    } else {
        //printf("%s shader %s loaded successfully\n", shader_kind_to_string(kind), path);
    }

    return this->init_from_string(kind, src.handle.contents);
}

void Shader::attach_to(ShaderProgram* program) {
    glAttachShader(program->id, this->id);
}

void Shader::detach_from(ShaderProgram* program) {
    glDetachShader(program->id, this->id);
}

void Shader::destroy() {
    glDeleteShader(this->id);
}

bool ShaderProgram::lateinit() {
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

bool ShaderProgram::init_from_strings(const char* vs, const char* fs) {
    id = glCreateProgram();
    if (!vshader.init_from_string(GL_VERTEX_SHADER, vs)) return false;
    if (!fshader.init_from_string(GL_FRAGMENT_SHADER, fs)) return false;
    return this->lateinit();
}

bool ShaderProgram::init_from_files(const char* vspath, const char* fspath) {
    id = glCreateProgram();
    if (!vshader.init_from_file(GL_VERTEX_SHADER, vspath)) return false;
    if (!fshader.init_from_file(GL_FRAGMENT_SHADER, fspath)) return false;
    return this->lateinit();
}

void ShaderProgram::use() {
    glUseProgram(this->id);
}

void ShaderProgram::upload_mat4(const char* var, glm::mat4 mat4) {
    GLuint varloc = glGetUniformLocation(id, var);
    glUniformMatrix4fv(varloc, 1, GL_FALSE, glm::value_ptr(mat4));
}

void ShaderProgram::upload_float(const char* var, float f) {
    GLuint varloc = glGetUniformLocation(id, var);
    glUniform1f(varloc, f);
}

void ShaderProgram::upload_ivec2(const char* var, glm::ivec2 ivec2) {
    GLuint varloc = glGetUniformLocation(id, var);
    glUniform2i(varloc, ivec2.x, ivec2.y);
}
