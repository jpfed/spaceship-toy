#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "stars.h"

extern float aspectRatio;

void starsDraw(struct GameObject *go) {
	if (go->type == STARFIELD) {
		struct StarField *sf = (struct StarField *)(go->objData);
		int i;
		double b;
		glBegin(GL_POINTS);
		for (i=0;i<NUM_STARS;i++) {
			b = sf->brightness[i];
			glColor3d(b,b,b);
			glVertex2d(sf->x[i],sf->y[i]);
		}
		glEnd();
	}
}

struct GameObject *initStars() {
	struct GameObject *go = gameObjectInit();
	struct StarField *sf = malloc(sizeof(struct StarField));
	
	int i;
	srand(i);
	for (i=0;i<NUM_STARS;i++) {
		sf->x[i]= aspectRatio*2*(rand()/((double)RAND_MAX)-0.5);
		sf->y[i]= 2*(rand()/((double)RAND_MAX)-0.5);
		sf->brightness[i] = rand()/((double)RAND_MAX);	
	}
	go->mass = 0.0;
	go->moi = 0.0;
	go->phased = 1;
	go->objData = (void*)sf;
	go->type = STARFIELD;
	go->draw = starsDraw;

	return go;
}
