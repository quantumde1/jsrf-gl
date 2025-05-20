#pragma once

#include <stdio.h>
#include <stdint.h>

#define MAX_MODELS 64
#define MAX_PLACEMENTS 1024

typedef struct {
    uint32_t model_index;
    float position[3];
    float rotation[3]; // В градусах
    float scale[3];
} Placement;

typedef struct {
    char filename[32];
    uint32_t index;
} ModelEntry;

typedef struct {
    ModelEntry models[MAX_MODELS];
    int model_count;

    Placement placements[MAX_PLACEMENTS];
    int placement_count;
} MapData;

typedef struct {
    uint8_t magic[4];       // 'BMAP'
    uint32_t version;       // 1
    uint32_t model_count;
    uint32_t placement_count;
} BMapHeader;

int load_bmap(const char* filename, MapData* map);