#pragma once

#ifdef _arch_dreamcast
#include "GL/glkos.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "kos.h"
#else
#include "GL/freeglut.h"
#endif

#include "stdint.h"

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float s, t;
} TexCoord;

typedef struct {
    char name[64];
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
    GLuint texture_id;
    char map_kd_path[128];
} Material;

typedef struct {
    unsigned int *indices;
    int count;
} IndexGroup;

typedef struct {
    float *vertices;
    float *texCoords;
    unsigned int *indices;
    unsigned int *faceMaterials;

    int vertexCount;
    int texCoordCount;
    int indexCount;

    Material *materials;
    int materialCount;

    IndexGroup *sortedIndices;
} Model;

typedef struct {
    float x, y, z; // Position
    float rx, ry, rz; // Rotation
    float sx, sy, sz; // Scale
    uint32_t model_id; // ID of the model to place
} MapEntity;

Model loadOBJ(const char *filename);
void drawModelEx(Model *model,
                 float posX, float posY, float posZ,
                 float rotX, float rotY, float rotZ,
                 float scaleX, float scaleY, float scaleZ,
                 float r, float g, float b, float a);

void freeModel(Model *model);