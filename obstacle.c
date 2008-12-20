#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "universe.h"
#include "obstacle.h"

/* Make obstacles explode! */

extern float aspectRatio;
#define OB_SIZE 0.025
struct GameObject* initObstacle() {
	struct GameObject *go = gameObjectInit();
	struct Obstacle* ob = malloc(sizeof(struct Obstacle));
	go->mass = GO_DENSITY*OB_SIZE*OB_SIZE;
	go->moi = OB_SIZE*OB_SIZE*go->mass/50;
	go->maxDim = 2*OB_SIZE;
	go->phased = 0;
	
	struct Polygon *poly = malloc(sizeof(struct Polygon));
	poly->numPoints = 4;
	poly->points = malloc(4*sizeof(struct Pair));
	poly->points[0].x = OB_SIZE; poly->points[0].y = OB_SIZE;
	poly->points[1].x = -OB_SIZE; poly->points[1].y = OB_SIZE;
	poly->points[2].x = -OB_SIZE; poly->points[2].y = -OB_SIZE;
	poly->points[3].x = OB_SIZE; poly->points[3].y = -OB_SIZE;
	go->bounds = poly;
	
	go->objData = (void*)ob;
	go->type = OBSTACLE;
	go->draw = obstacleDraw;
	go->simulate = obstacleSimulate;
	return go;
}

void obstacleDraw(struct GameObject *go) {
	glPushMatrix();
	glTranslated(go->pos.x, go->pos.y,0);
	glRotated(go->angle,0,0,1);
	glBegin(GL_POLYGON);
	glColor3d(0,0.75,0.25);
	glVertex2d(OB_SIZE,OB_SIZE);
	glVertex2d(-OB_SIZE,OB_SIZE);
	glVertex2d(-OB_SIZE,-OB_SIZE);
	glVertex2d(OB_SIZE,-OB_SIZE);	
	glEnd();
	glPopMatrix();
}

void obstacleSimulate(struct GameObject *go, double dt) {
	if (go->type == OBSTACLE) {
		applyMomentum(go,dt);
		toroidalWrap(go,2*aspectRatio,2);
	}
}
