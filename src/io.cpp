typedef struct {
    const char* path;
    const char* abs_path;
    const char* contents;
    usize len;
} File;

typedef enum {
    FILEOP_FAILURE,
    FILEOP_SUCCESS,
} FileOpResult;

typedef struct {
    File handle;
    FileOpResult status;
} FileOrError;

FileOrError read_file(const char* path) {
    // TODO: more thorough error checking
    FILE* raw = fopen(path, "r");
    if (!raw) {
        return (FileOrError){ {}, FILEOP_FAILURE };
    }

    fseek(raw, 0, SEEK_END);
    usize size = ftell(raw);
    rewind(raw);

    char* contents = (char*)malloc(size + 1);
    fread(contents, sizeof(char), size, raw);
    fclose(raw);
    contents[size] = '\0';

    char abs_path_buf[PATH_MAX + 1];
    char* abs_path = NULL;
    if (realpath(path, abs_path_buf)) {
        usize abs_path_len = strlen(abs_path_buf);
        abs_path = (char*)malloc(abs_path_len + 1);
        memcpy(abs_path, abs_path_buf, abs_path_len+1); // including '\0'
    }

    File handle;
    handle.path = path;
    handle.abs_path = abs_path;
    handle.contents = contents;
    handle.len = size;
    return (FileOrError){ handle, FILEOP_SUCCESS };
}
