#pragma once
#include <stdint.h>
#include "hash.h"

struct Hash *gameObjects;

void objectInit();

void handleInput(Uint8 *);

void simulateWorld();

void drawScene();
