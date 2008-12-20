#pragma once
#include <stdint.h>
#include "coreutils.h"
#define STARFIELD 3

#define NUM_STARS 256
struct StarField {
	double x[NUM_STARS];
	double y[NUM_STARS];
	double brightness[NUM_STARS];

};

struct GameObject* initStars();
void starsDraw(struct GameObject*);
void starsSimulate(struct GameObject*);
void starsInput(struct GameObject*, Uint8 *);
void starsCollide(struct GameObject*,struct GameObject*);
