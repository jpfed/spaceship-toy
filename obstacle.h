#pragma once
#include <stdint.h>
#include "coreutils.h"
#define OBSTACLE (5 | COLLISION_SENSITIVE)

struct Obstacle {
	double shield;
};

struct GameObject* initObstacle();
void obstacleDraw(struct GameObject*);
void obstacleSimulate(struct GameObject*, double);
