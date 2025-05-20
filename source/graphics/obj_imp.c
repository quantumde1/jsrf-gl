#ifdef _arch_dreamcast
#include "GL/glkos.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "kos.h"
#else
#include "GL/freeglut.h"
#endif

#include "stdio.h"
#include <stdlib.h>
#include "string.h"

#include "../../include/obj_imp.h"
#include "../../include/md2_imp.h"
#include "../../include/abstraction.h"

// Material loading logic
Material* loadMTL(const char *filename, int *materialCount) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        writeln("Failed to open MTL file: %s\n", filename);
        *materialCount = 0;
        return NULL;
    }

    Material *materials = malloc(16 * sizeof(Material));
    int capacity = 16, count = 0;
    char line[128];

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "newmtl ", 7) == 0) {
            if (count >= capacity) {
                capacity *= 2;
                materials = realloc(materials, capacity * sizeof(Material));
            }
            sscanf(line + 7, "%s", materials[count].name);
            materials[count].shininess = 0.0f;
            materials[count].texture_id = 0;
            materials[count].map_kd_path[0] = '\0';
        } else if (strncmp(line, "map_Kd ", 7) == 0) {
            sscanf(line + 7, "%s", materials[count].map_kd_path);
            char fullPath[256];
            #ifdef _arch_dreamcast
            snprintf(fullPath, sizeof(fullPath), concat_strings(ASSETS_PATH, "%s"), materials[count].map_kd_path);
            #else
            snprintf(fullPath, sizeof(fullPath), concat_strings(ASSETS_PATH, "%s"), materials[count].map_kd_path);
            #endif
            materials[count].texture_id = loadTexture(fullPath);
            count++;
        }
    }

    fclose(file);
    *materialCount = count;
    return materials;
}

Model loadOBJ(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        writeln("Failed to open OBJ file: %s\n", filename);
        exit(1);
    }

    Model model = {0};
    char mtlFile[256] = "";

    // count tex vertices, faces, indexes
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) model.vertexCount++;
        else if (strncmp(line, "vt ", 3) == 0) model.texCoordCount++;
        else if (strncmp(line, "f ", 2) == 0) model.indexCount += 3;
        else if (strncmp(line, "mtllib ", 7) == 0) sscanf(line + 7, "%s", mtlFile);
    }
    rewind(file);

    // mem allocation
    model.vertices = malloc(model.vertexCount * 3 * sizeof(float));
    model.texCoords = malloc(model.texCoordCount * 2 * sizeof(float));
    model.indices = malloc(model.indexCount * sizeof(unsigned int));
    model.faceMaterials = calloc(model.indexCount / 3, sizeof(unsigned int));

    // loading materials
    if (mtlFile[0]) {
        char fullPath[256];
        #ifdef _arch_dreamcast
            snprintf(fullPath, sizeof(fullPath), concat_strings(ASSETS_PATH, "%s"), mtlFile);
        #else
            snprintf(fullPath, sizeof(fullPath), concat_strings(ASSETS_PATH, "%s"), mtlFile);
        #endif
        model.materials = loadMTL(fullPath, &model.materialCount);
    }

    int vIndex = 0, vtIndex = 0, fIndex = 0, faceIndex = 0;
    int currentMaterial = 0;

    // parse OBJ
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            sscanf(line, "v %f %f %f",
                   &model.vertices[vIndex * 3],
                   &model.vertices[vIndex * 3 + 1],
                   &model.vertices[vIndex * 3 + 2]);
            vIndex++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            sscanf(line, "vt %f %f",
                   &model.texCoords[vtIndex * 2],
                   &model.texCoords[vtIndex * 2 + 1]);
            model.texCoords[vtIndex * 2 + 1] = 1.0f - model.texCoords[vtIndex * 2 + 1];
            vtIndex++;
        } else if (strncmp(line, "usemtl ", 7) == 0) {
            char matName[64];
            sscanf(line + 7, "%s", matName);
            for (int i = 0; i < model.materialCount; ++i) {
                if (strcmp(model.materials[i].name, matName) == 0) {
                    currentMaterial = i;
                    break;
                }
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            unsigned int v[3], vt[3];
            sscanf(line, "f %u/%u %u/%u %u/%u",
                   &v[0], &vt[0],
                   &v[1], &vt[1],
                   &v[2], &vt[2]);

            for (int i = 0; i < 3; ++i) {
                model.indices[fIndex++] = v[i] - 1;
            }
            model.faceMaterials[faceIndex++] = currentMaterial;
        }
    }

    fclose(file);

    // sort indicies by material
    model.sortedIndices = calloc(model.materialCount, sizeof(IndexGroup));
    for (int i = 0; i < model.materialCount; ++i) {
        model.sortedIndices[i].indices = malloc(model.indexCount * sizeof(unsigned int));
        model.sortedIndices[i].count = 0;
    }

    for (int i = 0; i < model.indexCount / 3; ++i) {
        int matID = model.faceMaterials[i];
        if (matID >= 0 && matID < model.materialCount) {
        memcpy(&model.sortedIndices[matID].indices[model.sortedIndices[matID].count * 3],
               &model.indices[i * 3], 3 * sizeof(unsigned int));
        model.sortedIndices[matID].count++;
        } else {
            writeln("Error at 148 line of OBJ loader");
        }
    }

    return model;
}

void drawModelEx(Model *model,
    float posX, float posY, float posZ,
    float rotX, float rotY, float rotZ,
    float scaleX, float scaleY, float scaleZ,
    float r, float g, float b, float a) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, model->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, model->texCoords);

    glPushMatrix();
    glTranslatef(posX, posY, posZ);
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotZ, 0.0f, 0.0f, 1.0f);
    glScalef(scaleX, scaleY, scaleZ);

    glColor4f(r, g, b, a);

    for (int m = 0; m < model->materialCount; m++) {
        Material *mat = &model->materials[m];
        glBindTexture(GL_TEXTURE_2D, mat->texture_id);

        int indexCount = model->sortedIndices[m].count * 3;
        if (indexCount > 0) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, model->sortedIndices[m].indices);
        }
    }

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void freeModel(Model *model) {
    free(model->vertices);
    free(model->texCoords);
    free(model->indices);
    free(model->faceMaterials);
    for (int i = 0; i < model->materialCount; ++i) {
        free(model->sortedIndices[i].indices);
    }
    free(model->sortedIndices);
    free(model->materials);
}