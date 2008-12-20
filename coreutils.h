#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)
#define CONVERT_TO_RAD 0.0174532925

/* **************************************************** */

#define PR struct Pair*
struct Pair {
	double x;
	double y;	
};

#define DOT_PRODUCT(P1,P2)		((P1.x)*(P2.x)+(P1.y)*(P2.y))
#define CROSS_PRODUCT(P1,P2) 	((P1.x)*(P2.y)-(P1.y)*(P2.x))
#define LENGTH(P1) 				sqrt((P1.x)*(P1.x)+(P1.y)*(P1.y))
#define LENGTH_SQ(P1) 			((P1.x)*(P1.x)+(P1.y)*(P1.y))
#define DISTANCE_SQUARED(P1,P2)	(((P1.x)-(P2.x))*((P1.x)-(P2.x))+((P1.y)-(P2.y))*((P1.y)-(P2.y)))

struct Pair projectAontoB(struct Pair a, struct Pair b);

struct Pair getRotator(double angle);

struct Pair rotateAbyB(struct Pair a, struct Pair b);

struct Pair addAtoB(struct Pair a, struct Pair b);

struct Pair getNormal(struct Pair a, struct Pair b);

double getLength(struct Pair a);

/* **************************************************** */

#define POLY struct Polygon*
struct Polygon {
	int numPoints;
	struct Pair *points;
};

/* **************************************************** */

#define EMPTY 					0
#define VISIBLE					0x80000000
#define CUSTOM_BEHAVIOR			0x40000000
#define USER_CONTROLLED			0x20000000
#define COLLISION_SENSITIVE		0x10000000

#define GO_DENSITY 20000.0

#define GP struct GameObject*
struct GameObject {
	int type;
	
	double angle;
	double angVel;

	struct Pair pos;
	struct Pair vel;

	double torque;
	double moi;
	double maxDim;

	struct Pair force;
	double mass;

	int phased;	
	struct Polygon *bounds;
	struct Polygon *cachedBounds;

	int allowDraw;

	void *objData;
	void (*draw)(struct GameObject *);
	void (*simulate)(struct GameObject *, double);
	void (*inputHandler)(struct GameObject *, Uint8 *);	
	void (*collisionHandler)(struct GameObject *, struct GameObject *);
};

int is_a(struct GameObject *go, int category);

void drawStub(struct GameObject *go);
void simulateStub(struct GameObject *go, double dt);
void inputStub(struct GameObject *go, Uint8 *keys);
void collisionStub(struct GameObject *go, struct GameObject *go2);

struct GameObject* gameObjectInit();
void applyMomentum(struct GameObject *, double dt);
int toroidalWrap(struct GameObject *, double, double);
void applyCooldown(int*);
