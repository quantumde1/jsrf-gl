#include "../../include/bmap_parser.h"
#include "../../include/abstraction.h"
#include "../../include/obj_imp.h"
#include "../../include/audio_playback.h"

#ifdef _arch_dreamcast
#include "GL/glkos.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "kos.h"
#else

int keyStates[256] = {0};

#include "GL/freeglut.h"
#include "GL/glu.h"
#endif

#include <math.h>

#define DEG2RAD(x) ((x) * M_PI / 180.0f)

float lastTime = 0.0f;

typedef struct {
    float x, y, z;     // Player position
    float rotation;     // rotation angle
    Model model;       //player model
} Player;

typedef struct {
    float distance;
    float angle;
    float height;
} Camera;

Player player = {0};
Camera camera = {5.0f, 0.0f, 2.0f};

#ifndef _arch_dreamcast
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = 1;
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = 0;
}

void handleInput() {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    float moveSpeed = 2.0f * deltaTime;
    float rotateSpeed = 1.5f * deltaTime;
    
    if (keyStates['w']) player.z += moveSpeed;
    if (keyStates['s']) player.z -= moveSpeed;
    if (keyStates['a']) player.x += moveSpeed;
    if (keyStates['d']) player.x -= moveSpeed;
    if (keyStates['q']) camera.angle += rotateSpeed;
    if (keyStates['e']) camera.angle -= rotateSpeed;
    if (keyStates['r']) camera.height += moveSpeed;
    if (keyStates['f']) camera.height -= moveSpeed;
}
#endif

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    // Инициализация игрока
    player.x = 0.0f;
    player.y = 0.0f;
    player.z = 0.0f;
    player.rotation = 0.0f;
    
    // Настройка проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

#ifdef _arch_dreamcast
void handleInput() {
    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont) {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
        if (state) {
            float moveX = state->joyx;
            float moveY = state->joyy;

            #define STICK_DEADZONE 16
            if (fabs(moveX) > STICK_DEADZONE || fabs(moveY) > STICK_DEADZONE) {
                float angleRad = atan2f(moveY, moveX);
                float magnitude = sqrtf(moveX*moveX + moveY*moveY) / 128.0f;

                if (magnitude > 1.0f) magnitude = 1.0f;

                float moveSpeed = magnitude * 0.1f;

                player.x -= cosf(angleRad) * moveSpeed;
                player.z -= sinf(angleRad) * moveSpeed;
            }

            if (state->rtrig > 64) {
                camera.angle += 0.03f;
            }
            if (state->rtrig > 32) {
                camera.angle += 0.015f;
            }
            if (state->ltrig > 32) {
                camera.angle -= 0.015f;
            }
            if (state->ltrig > 64) {
                camera.angle -= 0.03f;
            }
        }
    }
}
#endif

void display() {
    // empty
}

int main(int argc, char **argv) {
    writeln("Game initialization\nJET SET RADIO!");
    glInit(&argc, argv);

    init();

    MapData map;
    Model loaded_models[MAX_MODELS] = {0};
    char* player_path = concat_strings(ASSETS_PATH, "rhyth.obj");
    player.model = loadOBJ(player_path);
    if (player.model.vertexCount == 0) {
        writeln("Failed to load player model: %s\n", player_path);
    }

    if (load_bmap(concat_strings(ASSETS_PATH, "map00.bmap"), &map) != 0) {
        printf("Failed to load map\n");
        return 1;
    }

    for (int i = 0; i < map.model_count; i++) {
        char* filepath = concat_strings(ASSETS_PATH, map.models[i].filename);
        loaded_models[i] = loadOBJ(filepath);
        if (loaded_models[i].vertexCount == 0) {
            writeln("failed to load model: %s\n", filepath);
        }
    }

    #ifndef _arch_dreamcast
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutIgnoreKeyRepeat(1);
    #else
    audioPlayback("buttrfly.adx");
    #endif

    while (1) {
        handleInput();
        #ifndef _arch_dreamcast
        glutMainLoopEvent();
        #endif
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        
        //camera rotation
        float camX = player.x + camera.distance * cosf(camera.angle);
        float camZ = player.z + camera.distance * sinf(camera.angle);
        float camY = player.y + camera.height;
        
        gluLookAt(camX, camY, camZ, // position of camera
                  player.x, player.y + 1.0f, player.z, //point of look
                  0.0f, 1.0f, 0.0f);//up
    
        // draw player
        drawModelEx(&player.model,
            player.x, player.y, player.z,
            0.0f, player.rotation, 0.0f,
            0.1f, 0.1f, 0.1f,
            1.0f, 1.0f, 1.0f, 1.0f);
    
        // draw map objects
        for (int i = 0; i < map.placement_count; i++) {
            Placement p = map.placements[i];
            drawModelEx(&loaded_models[i],
                        p.position[0], p.position[1], p.position[2],
                        p.rotation[0], p.rotation[1], p.rotation[2],
                        p.scale[0], p.scale[1], p.scale[2],
                        1.0f, 1.0f, 1.0f, 1.0f);
        }
    
        glSwapBuffers();
    }

    freeModel(&player.model);
    for (int i = 0; i < map.placement_count; i++) {
        freeModel(&loaded_models[i]);
    }

    return 0;
}