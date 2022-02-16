#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <glad/glad.h>

bool compile_shader_file(int shader, const char* path)
{
    #define SHADER_SOURCE_BUFFER_SIZE 1000
    char shader_source_buffer[SHADER_SOURCE_BUFFER_SIZE];

    int success = true;

    FILE *source_file = fopen(path, "r");
    if (source_file)
    {
        size_t length = fread(shader_source_buffer, sizeof(char), SHADER_SOURCE_BUFFER_SIZE - 1, source_file);

        if (length == SHADER_SOURCE_BUFFER_SIZE - 1) {
            fputs("SHADER_SOURCE_BUFFER overflow", stderr);
        }

        if (!ferror(source_file))
        {
            shader_source_buffer[length] = '\0';

            const char* source[1] = { shader_source_buffer };
            glShaderSource(shader, 1, source, NULL);
            glCompileShader(shader);

            int compilation_successful;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_successful);

            if (!compilation_successful)
            {
                #define ERROR_LOG_BUFFER_SIZE 512
                char error_log_buffer[ERROR_LOG_BUFFER_SIZE];
                glGetShaderInfoLog(shader, ERROR_LOG_BUFFER_SIZE, NULL, error_log_buffer);

                int shader_type;
                glGetShaderiv(shader, GL_SHADER_TYPE, &shader_type);
                const char* shader_type_string = (shader_type == GL_VERTEX_SHADER) ? "vertex" : "fragment";

                fprintf(stderr, "%s shader compilation error:\n%s\n", shader_type_string, error_log_buffer);
            }

            fclose(source_file);
        }
        else
        {
            fputs("Error reading from shader source file", stderr);
            success = false;
        }
    }
    else
    {
        fputs("Couldn't open shader source file", stderr);
        success = false;
    }

    return success;
}

#endif
