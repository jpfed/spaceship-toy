#pragma once
#include <stdint.h>
#include "coreutils.h"
#define BULLET (2 | COLLISION_SENSITIVE) 

struct Bullet {
	double speed;
	int timer;
	int explodeFrame;
	int trigger;
	struct GameObject *explosion;
};

struct GameObject* initBullet();
void bulletDraw(struct GameObject*);
void bulletSimulate(struct GameObject*, double);
void bulletCollide(struct GameObject*,struct GameObject*);
