#include "../../include/bmap_parser.h"
#include "../../include/abstraction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// secure reading of line
int read_string(FILE* f, char* buffer, size_t max_len) {
    if (!f || !buffer || max_len == 0) {
        return -1; // incorrent arguments
    }

    uint32_t length = 0;

    if (fread(&length, sizeof(uint32_t), 1, f) != 1) {
        return -1; // error length reading
    }

    if (length >= max_len) {
        length = max_len - 1; // guard from buffer overflow
    }

    if (fread(buffer, sizeof(char), length, f) != length) {
        return -1; // error reading data
    }

    buffer[length] = '\0'; // end of line
    return 0;
}

// secure reading of vector3
int read_vec3(FILE* f, float* vec) {
    if (!f || !vec) {
        return -1;
    }

    if (fread(vec, sizeof(float), 3, f) != 3) {
        return -1; // reading error
    }

    return 0;
}

// main .bmap loader
int load_bmap(const char* filename, MapData* map) {
    if (!filename || !map) {
        return -1;
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        writeln("Cannot open file: %s", filename);
        return -1; // cannot open file
    }

    BMapHeader header;

    // reader header
    if (fread(&header, sizeof(BMapHeader), 1, f) != 1) {
        writeln("Error: header is not correct");
        fclose(f);
        return -1;
    }

    // check signature and version
    if (memcmp(header.magic, "BMAP", 4) != 0 || header.version != 1) {
        writeln("Error: header version not correct");
        fclose(f);
        return -1;
    }

    // setting map data to zero
    memset(map, 0, sizeof(MapData));

    // --- Model reading ---
    for (uint32_t i = 0; i < header.model_count; ++i) {
        if (map->model_count >= MAX_MODELS) {
            fclose(f);
            return -1; // too much data
        }

        uint32_t index;

        if (fread(&index, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -1;
        }

        ModelEntry* model = &map->models[map->model_count];

        if (read_string(f, model->filename, sizeof(model->filename)) != 0) {
            fclose(f);
            return -1;
        }

        model->index = index;
        map->model_count++;
    }

    // --- read placements ---
    for (uint32_t i = 0; i < header.placement_count; ++i) {
        if (map->placement_count >= MAX_PLACEMENTS) {
            fclose(f);
            return -1; // too much data
        }

        Placement p;

        if (fread(&p.model_index, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -1;
        }

        if (read_vec3(f, p.position) != 0 ||
            read_vec3(f, p.rotation) != 0 ||
            read_vec3(f, p.scale) != 0) {
            fclose(f);
            return -1;
        }

        map->placements[map->placement_count++] = p;
    }

    fclose(f);
    return 0; // gracefully exit
}