#pragma once
#include <stdint.h>
#include "coreutils.h"
#define PARTICLESYS 4

#define NUM_PARTS 256
struct ParticleSystem {
	double x[NUM_PARTS];
	double xV[NUM_PARTS];
	double y[NUM_PARTS];
	double yV[NUM_PARTS];
	double life[NUM_PARTS];
	double pSize[NUM_PARTS];
	double intensity;
	double indFade;
	double overallFade;
	int cursor;
	int inward;
	double rMax;
	double rMin;
	double gMax;
	double gMin;
	double bMax;
	double bMin;
	double size;
	double sizeDev;
	double angleDev;
	double speed;
	double speedDev;
};

struct GameObject* initParticles();
void particlesDraw(struct GameObject*);
void particlesSimulate(struct GameObject*, double);

