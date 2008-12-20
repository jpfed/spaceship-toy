#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "spacew.h"
#include "universe.h"
#include "particles.h"
#include "bullet.h"

#define BULLET_INITIAL_TIMER 400
#define BULLET_EXPLOSION_FRAMES 20
#define BULLET_PHASE 2
#define BULLET_SIZE 0.005

/* 

Make the logic into an explicit DFA with just a few extra variables
Get rid of timer?  Eventually there will be enough going on that we won't need it.

Make a few more weapons-
	laser (straight line that fades rapidly)
	ion beam (particles of destruction)
	warp wave (expanding circle of destruction, weak at larger radius)
	nuke (bullet-like, but in 2 seconds or so it blows up with an AOE)
	drone (guided weapon that seeks the closest object that's not the firer)
*/
extern float aspectRatio;

void freeBullet(struct GameObject *go) {
	if (!hashRemove(gameObjects, (void*)go)) exit(1);
	
	struct Bullet *bullet = (struct Bullet *)(go->objData);
	hashRemove(gameObjects, bullet->explosion);
	free(bullet->explosion);
	free(go->objData);
	free(go->bounds->points);
	free(go->bounds);
	free(go);
}

void bulletDraw(struct GameObject *go) {
	if (go->type == BULLET) {
		struct Bullet *bullet = (struct Bullet *)(go->objData);
		glPushMatrix();
		glTranslated(go->pos.x, go->pos.y,0);
		double rad = BULLET_SIZE;
		double mrad = 0.7071*rad;
		if (bullet->trigger == 0) {
			double b = bullet->timer/(double)BULLET_INITIAL_TIMER;
			glColor3d(b,b,1.0);
			glBegin(GL_POLYGON);
		}
		else {
			glColor3d(1,bullet->explodeFrame/(double)BULLET_EXPLOSION_FRAMES,0);
			glBegin(GL_LINE_LOOP);
		}
		glVertex2d(0,rad);
		glVertex2d(-mrad,mrad);
		glVertex2d(-rad,0);
		glVertex2d(-mrad,-mrad);
		glVertex2d(0,-rad);
		glVertex2d(mrad,-mrad);
		glVertex2d(rad,0);
		glVertex2d(mrad,mrad);
		glEnd();
		glPopMatrix();
	}
}

void triggerBullet(struct Bullet *bullet) {
	if (bullet->trigger==0) {
		bullet->explodeFrame = BULLET_EXPLOSION_FRAMES;
		bullet->timer = -1;
		struct ParticleSystem *exp = (struct ParticleSystem *)(bullet->explosion->objData);
		exp->intensity = 0.95;
		exp->overallFade = 0.95;
		exp->indFade = 0.75;
		exp->speed = 2.0;
		exp->speedDev = 2.0;
	}
	bullet->trigger = 1;
	
}



void bulletSimulate(struct GameObject *go, double dt) {
	if (go->type == BULLET) {
		struct Bullet *bullet = (struct Bullet *)(go->objData);

		applyMomentum(go,dt);
		toroidalWrap(go,2*aspectRatio,2);
		
		bullet->timer--;
		
		if (bullet->trigger == 1) {
			bullet->explodeFrame--;
		}	
		else {
			if (go->phased>0) go->phased--;
			if (bullet->timer==0) 
				triggerBullet(bullet);
		}
		if ((bullet->trigger==1) && (bullet->explodeFrame <= 0)) {
			freeBullet(go);
		}
		
		bullet->explosion->angle = go->angle;
		bullet->explosion->pos = go->pos;
		bullet->explosion->vel = go->vel;
		
	}
}

 
void bulletCollide(struct GameObject* go, struct GameObject *other) {
	struct Bullet *bullet = ((struct Bullet *)(go->objData));
	triggerBullet(bullet);
	go->phased = 20;
}

struct GameObject *initBullet() {
	struct GameObject *go = gameObjectInit();
	struct Bullet *bullet = malloc(sizeof(struct Bullet));
	bullet->explodeFrame = 0;
	bullet->timer = BULLET_INITIAL_TIMER;
	bullet->speed = 0.6;
	bullet->trigger = 0;
	
	bullet->explosion = initParticles(go);
	hashAdd(gameObjects,bullet->explosion);
	struct ParticleSystem *exp = (struct ParticleSystem *)(bullet->explosion->objData);
	exp->intensity = 0;
	exp->overallFade = 1.0;
	exp->angleDev = 720;
	
	go->mass = GO_DENSITY*BULLET_SIZE*BULLET_SIZE;
	go->moi = BULLET_SIZE*BULLET_SIZE*go->mass;
	go->maxDim = 1.4*BULLET_SIZE;
	go->phased = BULLET_PHASE;
	

	struct Polygon *poly = malloc(sizeof(struct Polygon));
	poly->numPoints = 4;
	poly->points = malloc(4*sizeof(struct Pair));
	poly->points[0].x = BULLET_SIZE; poly->points[0].y = BULLET_SIZE;
	poly->points[1].x = -BULLET_SIZE; poly->points[1].y = BULLET_SIZE;
	poly->points[2].x = -BULLET_SIZE; poly->points[2].y = -BULLET_SIZE;
	poly->points[3].x = BULLET_SIZE; poly->points[3].y = -BULLET_SIZE;
	go->bounds = poly;
	
	go->objData = (void*)bullet;
	go->type = BULLET;
	go->draw = bulletDraw;
	go->simulate = bulletSimulate;
	go->collisionHandler = bulletCollide;
	return go;
}


