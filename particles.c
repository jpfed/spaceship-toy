#include<stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "particles.h"


/* Include the option to free itself if its intensity drops too low*/

extern float aspectRatio;

struct GameObject* initParticles() {
	struct GameObject *go = gameObjectInit();
	struct ParticleSystem *ps = malloc(sizeof(struct ParticleSystem));
	
	int i;
	for (i=0;i<NUM_PARTS;i++) {
		ps->x[i]=0;
		ps->xV[i]=0;
		ps->y[i]=0;
		ps->yV[i]=0;
		ps->life[i]=0;	
		ps->pSize[i]=0;		
	}
	
	ps->intensity=1.0;
	ps->indFade=0.975;
	ps->overallFade=0.975;
	ps->cursor=0;
	ps->inward=0;
	ps->rMax=1;
	ps->rMin=1;
	ps->gMax=1;
	ps->gMin=0;
	ps->bMax=1;
	ps->bMax=0;
	ps->size=0.004;
	ps->sizeDev=0.003;
	ps->angleDev = 45;
	ps->speed = 0.6;
	ps->speedDev = 0.05;
	go->phased = 1;
	go->objData = (void *)ps;
	go->type = PARTICLESYS;
	go->draw = particlesDraw;
	go->simulate = particlesSimulate;
	return go;
}

void particlesDraw(struct GameObject* go) { 
	if (go->type == PARTICLESYS) {
		struct ParticleSystem *ps = (struct ParticleSystem *)(go->objData);
		glBegin(GL_QUADS);
		int i;
		double r,g,b,cf,sz;
		for (i=0;i<NUM_PARTS;i++) {
			cf = ps->life[i];
			r = ps->rMax*cf+(1-cf)*ps->rMin;
			g = ps->gMax*cf+(1-cf)*ps->gMin;
			b = ps->bMax*cf+(1-cf)*ps->bMin;
			
			glColor4d(r,g,b,cf);
			
			sz = ps->pSize[i];
			
			glVertex2d(ps->x[i]+sz,ps->y[i]+sz);
			glVertex2d(ps->x[i]-sz,ps->y[i]+sz);
			glVertex2d(ps->x[i]-sz,ps->y[i]-sz);
			glVertex2d(ps->x[i]+sz,ps->y[i]-sz);
		}
		glEnd();
	}
	
}


void particlesSimulate(struct GameObject* go,double dt) {
	if (go->type == PARTICLESYS) {
		struct ParticleSystem *ps = (struct ParticleSystem *)(go->objData);
		int i;
		double diff;
		for (i=0;i<NUM_PARTS;i++) {
			if (ps->inward) {
				diff = go->pos.x - ps->x[i];
				ps->x[i]+= 0.25*diff;
				diff = go->pos.y - ps->y[i];
				ps->y[i]+= 0.25*diff;
			}
			else {
				ps->x[i]+=dt*ps->xV[i];
				ps->y[i]+=dt*ps->yV[i];
			
				if (ps->x[i]<-aspectRatio) ps->x[i]+=2*aspectRatio;
				if (ps->y[i]<-1) ps->y[i]+=2;
				if (ps->x[i]>aspectRatio) ps->x[i]-=2*aspectRatio;
				if (ps->y[i]>1) ps->y[i]-=2;
			}
			if (!ps->inward) ps->life[i]*=ps->indFade;
			else {
				if (ps->life[i] >= 1.0) ps->life[i] = 0.0;
				ps->life[i]/=ps->indFade;
			}
		}
		ps->intensity*=ps->overallFade;
		
		int cursor = ps->cursor;
		int stopIndex = (cursor+NUM_PARTS-1)%NUM_PARTS;
		double angle, angle1, angle2, speed;
		while ((rand()/(double)RAND_MAX)<ps->intensity && cursor!=stopIndex) {
			i = ps->cursor;
			angle1 = go->angle+((rand()/(double)RAND_MAX)-0.5)*ps->angleDev;
			angle2 = go->angle+((rand()/(double)RAND_MAX)-0.5)*ps->angleDev;
			angle = (angle1+angle2)/2;
			speed = ps->speed+((rand()/(double)RAND_MAX)-0.5)*ps->speedDev;
			if (!ps->inward) {
				ps->xV[i] = -cos(CONVERT_TO_RAD*angle2)*speed + go->vel.x;
				ps->yV[i] = -sin(CONVERT_TO_RAD*angle2)*speed + go->vel.y;
				ps->x[i] = go->pos.x; ps->y[i] = go->pos.y;	
				ps->life[i] = ps->intensity;
			}
			else {
				ps->xV[i] = cos(CONVERT_TO_RAD*angle2)*speed - go->vel.x;
				ps->yV[i] = sin(CONVERT_TO_RAD*angle2)*speed - go->vel.y;
				ps->x[i] = go->pos.x-ps->xV[i]*0.25; ps->y[i] = go->pos.y-ps->yV[i]*0.25;
				ps->life[i] = ps->intensity*pow(ps->indFade,0.25/dt);
			}
			ps->pSize[i] = ps->size + ((rand()/(double)RAND_MAX)-0.5)*((rand()/(double)RAND_MAX)-0.5)*ps->sizeDev;
			ps->cursor = (ps->cursor + 1) % NUM_PARTS;
			cursor=ps->cursor;
		}
	}
}

