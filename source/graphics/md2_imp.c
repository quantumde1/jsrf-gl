#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "../../include/md2_imp.h"
#include "../../include/abstraction.h"

#ifdef _arch_dreamcast
#include "GL/glkos.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "kos.h"
#else
#include "GL/freeglut.h"
#endif

#include "stdio.h"

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data;

    data = stbi_load(filename, &width, &height, &channels, STBI_rgb);
    
    if (!data) {
        fprintf(stderr, "error: cannot load texture: %s\n", filename);
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data); // freeing memory

    return texture_id;
}