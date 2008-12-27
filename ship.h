#pragma once
#include <stdint.h>
#include "coreutils.h"
#define SHIP (1  | COLLISION_SENSITIVE)

struct Ship {
	double weapon;
	double shield;
	double speed;
	double warpLimit;
	double turnSpeed;
	double currentTorque;
	int thrusters;
	int retro;
	int turnLeft;
	int turnRight;
	int firingBullet;
	int heat;
	int tempInvince;
	int engageTractor;
	int releaseTractor;
	int tractor;
	double tractorLength;
	struct GameObject *engines;
	struct GameObject *rengines;
	struct GameObject *tractorBeam;
	struct GameObject *tractoredObject;
};

struct GameObject* initShip();
void shipDraw(struct GameObject*);
void shipSimulate(struct GameObject*, double dt);
void shipInput(struct GameObject*, Uint8 *);
void shipCollide(struct GameObject*,struct GameObject*);

