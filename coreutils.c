#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "coreutils.h"

int is_a(struct GameObject *go, int category) {
	return ((go->type) & category);
}

struct Pair projectAontoB(struct Pair a, struct Pair b) {
	double lengthSqB = LENGTH_SQ(b);
	double dot = DOT_PRODUCT(a,b)/lengthSqB;
	b.x*=dot; b.y*=dot;
	return b;	
}

struct Pair getRotator(double angle) {
	struct Pair result;
	result.x = cos(CONVERT_TO_RAD*angle);
	result.y = sin(CONVERT_TO_RAD*angle);
	return result;
}

struct Pair rotateAbyB(struct Pair a, struct Pair b) {
	double tempX = a.x; double tempY=a.y;
	a.x = tempX*b.x - tempY*b.y;
	a.y = tempX*b.y + tempY*b.x;
	return a;
}

struct Pair addAtoB(struct Pair a, struct Pair b) {
	a.x+=b.x;
	a.y+=b.y;
	return a;
}

struct Pair AminusB(struct Pair a, struct Pair b) {
	a.x-=b.x;
	a.y-=b.y;
	return a;	
}

double getLength(struct Pair a) {
	return sqrt((a.x)*(a.x) + (a.y)*(a.y));	
}

struct Pair getNormal(struct Pair a, struct Pair b) {
	struct Pair result;
	double len = sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
	result.x = (b.y - a.y)/len;
	result.y = (a.x - b.x)/len;
	return result;	
}

void drawStub(struct GameObject *go) {}
void simulateStub(struct GameObject *go, double dt) {}
void inputStub(struct GameObject *go, Uint8 *keys) {}
void collisionStub(struct GameObject *go, struct GameObject *go2) {}

struct GameObject* gameObjectInit() {
	struct GameObject* result = malloc(sizeof(struct GameObject));
	result->type = EMPTY;

	result->angle =0;
	result->angVel=0;
	result->pos.x=0; result->pos.y=0;
	result->vel.x=0; result->vel.y=0;

	result->torque=0;
	result->moi = 200;
	result->force.x = 0;result->force.y = 0;	
	result->mass = 200;
	result->phased = 0;

	result->bounds = malloc(sizeof(struct Polygon));
	result->bounds->numPoints = 0;
	result->bounds->points = NULL;
	
	
	result->cachedBounds = malloc(sizeof(struct Polygon));
	result->cachedBounds->numPoints = 0;
	result->cachedBounds->points = NULL;
	
	result->allowDraw = 1;
	
	result->objData = NULL;
	result->draw = drawStub;
	result->simulate = simulateStub;
	result->inputHandler = inputStub;
	result->collisionHandler = collisionStub;
	
	return result;
}

void applyMomentum(struct GameObject* object,double dt) {
	double speed,angSpeed;
	
	
	object->angVel	+= dt*(object->torque / object->moi);
	angSpeed = abs(object->angVel);
	
	if (angSpeed>30/dt) {
		object->angVel*=0.9;
	} else if (angSpeed>3/dt) {
		object->angVel*=0.99;	
	} else if (angSpeed>0.3/dt) {
		object->angVel*=0.999;	
	}
	
	object->angle	+= dt*object->angVel;					
	object->vel.x	+= dt*(object->force.x/object->mass);
	object->vel.y	+= dt*(object->force.y/object->mass);
	speed = getLength(object->vel);
	if (speed>0.1/dt) {
		object->vel.x*=0.9;
		object->vel.y*=0.9;
	}
	else if (speed>0.01/dt) {
		object->vel.x*=0.99;
		object->vel.y*=0.99;	
	}
	else if (speed>0.001/dt) {
		object->vel.x*=0.999;
		object->vel.y*=0.999;	
	}
	object->pos.x	+= dt*(object->vel.x);					
	object->pos.y	+= dt*(object->vel.y);					
	object->force.x = 0;							
	object->force.y = 0;							
	object->torque = 0;								
}

int toroidalWrap(struct GameObject* object, double x_screen_size, double y_screen_size) {
	int result = 0;
	while (object->pos.x > x_screen_size/2) {object->pos.x-=x_screen_size; result = 1;}	
	while (object->pos.y > y_screen_size/2) {object->pos.y-=y_screen_size;	result = 1;}							
	while (object->pos.x < -x_screen_size/2) {object->pos.x+=x_screen_size;	result = 1;}	
	while (object->pos.y < -y_screen_size/2) {object->pos.y+=y_screen_size;	result = 1;}
	return result;					
}

void applyCooldown(int *var) {
	if ((*var)>0) (*var)--;
}
