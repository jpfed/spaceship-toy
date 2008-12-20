#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "spacew.h"
#include "ship.h"
#include "stars.h"
#include "obstacle.h"

extern float aspectRatio;


/* 

Priorities: 

	object pools
	
	organize source files into folders
		graphics libs
			common graphical tasks (e.g. textured squares, draw to offscreen buffer)
			heads up display
		sound libs
			what is the SDL sound architecture like? research this.
		math libs
			the mathy parts of coreutils
		option screens
			startup screen
			video options
			audio options
			control config
			new/ resume game
		game objects
			the GameObject part of coreutils
			user-controllable ships
			enemies
			weapons
			powerups
			rooms
		game scripting and control
			camera control
			interface mode state machine
			plot-point master state machine
			plot-point transitions


GameObject.type should have a few bitmasks defined for easy checking; if (gameObject->type & WEAPON_MASK) then we're dealing with a weapon of some kind.

What are the object categories?  We have about 24 bits of mask to use before we come close to interfering with all the possible objects we might want to make...
4 bits used so far for VISIBLE, CUSTOM_BEHAVIOR, USER_CONTROLLED, COLLISION_SENSITIVE
 
Rather than doing so many little mallocs, we DEFINITELY want to make object pools for common objects.
In fact, we can have an object pool for GameObjects and for every kind of object they might point to (bullet, enemies, etc.)

After we figure out how we want to deal with object tracking/freeing, make collisions emit blue sparks.  Could easily have a pool for this (a la ship.c)

speed collision detection!

LONG TERM: 
	this file will likely have to cooperate with input abstraction
	files for object data/ preferences
	title/ option screen at the beginning: selecting different kinds of ship?
	enemies
	minimap
	room file that can read a room description from disk and administer the walls and objects
	camera file to make the camera follow the player up to the limits of the room
	director file to script events/ plotline
*/



void GO_hashInit() {
	gameObjects = hashInit(256);
	gameObjects->numBuckets = 255;	
}

void objectInit() {
	GO_hashInit();
	struct GameObject *ship = initShip();
	struct GameObject *stars = initStars();
	struct GameObject *ob[16]; 
	int i;
	srand(i);
	for (i=0;i<16;i++) {
		ob[i] = initObstacle();
		ob[i]->pos.x = 2*aspectRatio*((rand()/(double)RAND_MAX)-0.5);
		ob[i]->pos.y = 2*((rand()/(double)RAND_MAX)-0.5);
		ob[i]->angle = 360*((rand()/(double)RAND_MAX)-0.5);
		ob[i]->angle = 0;
		hashAdd(gameObjects, ob[i]);
	}
	
	hashAdd(gameObjects, ship);
	hashAdd(gameObjects, stars);

}

void handleInput(Uint8 *keys) {
	struct HashElement *cursor = NULL;
	int i;
	for (i=0;i<gameObjects->numBuckets;i++) {
		cursor = gameObjects->buckets[i].head;
		while(cursor!=NULL) {
			((GP)(cursor->obj))->inputHandler((GP)(cursor->obj), keys);
			cursor = cursor->next;
		}
	}	
}

void buildBoundsCache(struct GameObject *go) {
	if (go==NULL) return;
	if (go->cachedBounds->points == NULL) {
		go->cachedBounds->numPoints = go->bounds->numPoints;
		go->cachedBounds->points = malloc(go->bounds->numPoints * sizeof(struct Pair));	
		if (go->cachedBounds->points == NULL) return;
	}
	
	struct Pair *poly = go->cachedBounds->points;
			
	struct Pair rotator;
	int i;
	rotator = getRotator(go->angle);
	
	for (i=0;i<go->cachedBounds->numPoints;i++) {
		poly[i] = rotateAbyB(go->bounds->points[i],rotator);
		poly[i] = addAtoB(poly[i],go->pos);
	}
	
}

int collision(struct GameObject *go1, struct GameObject *go2, struct Pair *hit, int *objIndex) {
	if (!is_a(go1,COLLISION_SENSITIVE) ||
		!is_a(go2,COLLISION_SENSITIVE) ||
		(go1->phased > 0) || 
		(go2->phased > 0) ||
		(go1->cachedBounds->numPoints == 0) ||
		(go2->cachedBounds->numPoints == 0)) return 0;
	
	int bound1 = go1->bounds->numPoints;
	int bound2 = go2->bounds->numPoints;
	
	struct Pair *poly[2];
	poly[0] = go1->cachedBounds->points;
	poly[1] = go2->cachedBounds->points;


	int pti,pti2,counter;
	int result = 0;
	double xinters; /* xcoordinate of intersection */
	struct Pair p,p1,p2;
	for (pti=0;pti<bound1;pti++) {
		p=poly[0][pti];
		counter = 0;
		p1 = poly[1][0];
		for (pti2=1;pti2<=bound2;pti2++) {
			p2 = poly[1][pti2%bound2];
			if ((p.y > MIN(p1.y,p2.y)) &&
			(p.y <= MAX(p1.y,p2.y)) &&
			(p.x <= MAX(p1.x,p2.x)) &&
			(p1.y != p2.y)) {
				xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
				if (p1.x == p2.x || p.x <= xinters) counter++;	
			}
			p1=p2;
		}
		if (counter%2==1) {
			result = 1;
			if (counter==1)
				*hit = p;
			else {
				hit->x = ((counter-1)*hit->x + p.x) / counter;
				hit->y = ((counter-1)*hit->y + p.y) / counter;
			}	
			*objIndex = 0;
		}	
	}
	
	for (pti=0;pti<bound2;pti++) {
		p=poly[1][pti];
		counter = 0;
		p1 = poly[0][0];
		for (pti2=1;pti2<=bound1;pti2++) {
			p2 = poly[0][pti2%bound1];
			if ((p.y > MIN(p1.y,p2.y)) &&
			(p.y <= MAX(p1.y,p2.y)) &&
			(p.x <= MAX(p1.x,p2.x)) &&
			(p1.y != p2.y)) {
				xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
				if (p1.x == p2.x || p.x <= xinters) counter++;	
			}
			p1=p2;
		}
		if (counter%2==1) {
			result = 1;
			if (counter==1)
				*hit = p;
			else {
				hit->x = ((counter-1)*hit->x + p.x) / counter;
				hit->y = ((counter-1)*hit->y + p.y) / counter;
			}	
			*objIndex = 1;
		}	
	}	
	
	return result;
}

double resolveCollisionPoint(struct GameObject go1d, struct GameObject go2d, struct Pair *hit, double dt, int *objIndex) {

	struct GameObject *go1 = &go1d;
	struct GameObject *go2 = &go2d;

	/* direction is initialized to "backwards" */
	/* magnitude is initialized to dt/2 */
	int iteration;
	int maxIterations = 16;
	double direction = -1.0;
	double magnitude = dt/2;
	double t=dt;
	for (iteration=0;iteration<maxIterations;iteration++) {
		go1->pos.x += direction*(go1->vel.x * magnitude);
		go1->pos.y += direction*(go1->vel.y * magnitude);
		go1->angle += direction*(go1->angVel * magnitude);
		
		go2->pos.x += direction*(go2->vel.x * magnitude);
		go2->pos.y += direction*(go2->vel.y * magnitude);
		go2->angle += direction*(go2->angVel * magnitude);		
		
		t+= direction*magnitude;
		
		buildBoundsCache(go1);
		buildBoundsCache(go2);
		if (collision(go1,go2,hit, objIndex)) {
			direction = -1.0;
		}
		else {
			direction = 1.0;
		}
		magnitude /= 2.0;
	}
	return t;
}

void determineCollisionSurface(struct GameObject *go1, struct GameObject *go2, struct Pair colPos, struct Pair *surf1, struct Pair *surf2, int objIndex) {
	struct Pair result[2];
	struct Pair *poly[2];
	poly[0] = go1->cachedBounds->points;
	poly[1] = go2->cachedBounds->points;
	int bounds[2];
	bounds[0] = go1->cachedBounds->numPoints;
	bounds[1] = go2->cachedBounds->numPoints;
	
	double p1x,p1y,p2x,p2y;
	double distance, A,B,C,m,n;
	double bestDistance = RAND_MAX;
	struct Pair p1, p2, rp1,rp2, proj;
	int i,p=0;
	p = 1-objIndex;	
	for (i=0;i<go1->cachedBounds->numPoints;i++) {
		p1 = poly[p][i];
		p2 = poly[p][(i+1)%bounds[p]];
		rp1.x = colPos.x - p1.x;
		rp1.y = colPos.y - p1.y;
		rp2.x = p2.x - p1.x;
		rp2.y = p2.y - p1.y;
		
		m = colPos.x;
		n = colPos.y;
			
		proj = projectAontoB(rp1,rp2);
		proj.x -= rp1.x;
		proj.y -= rp1.y;
		distance = getLength(proj);
		
		if (distance < bestDistance) {
			bestDistance = distance;
			result[0] = poly[p][i];
			result[1] = poly[p][(i+1)%bounds[0]];
		}
	}
	
	*surf1 = result[0];
	*surf2 = result[1];
}

void simulateWorld() {
	static double dt = 1.0/60.0;
	int i,j;
	struct HashElement *cursor = NULL;
	struct HashElement *other = NULL;
	struct HashElement *next = NULL;
	struct GameObject *go1, *go2; 
	struct Pair colPos;
	
	for (i=0;i<gameObjects->numBuckets;i++) {
		cursor = gameObjects->buckets[i].head;
		while(cursor!=NULL) {
			buildBoundsCache((GP)cursor->obj);
			cursor = cursor->next;	
		}	
	}
	

	
	struct GameObject *objs[2];
	for (i=0;i<gameObjects->numBuckets;i++) {
		cursor = gameObjects->buckets[i].head;
		while(cursor!=NULL) {
			objs[0] = (GP)(cursor->obj);
			if (objs[0]->phased==0)
			for (j=i;j<gameObjects->numBuckets;j++) {
				if (j!=i) other = gameObjects->buckets[j].head;
				else other = cursor->next;
				while (other!=NULL) {
					objs[1] = (GP)(other->obj);
					int objIndex=0;
					if (collision(objs[0], objs[1], &colPos, &objIndex)) {
						double colDt = resolveCollisionPoint(*objs[0],*objs[1],&colPos,dt, &objIndex);
						
						struct Pair surface[2];
						determineCollisionSurface(objs[0],objs[1],colPos,&surface[0],&surface[1], objIndex);						
						
						struct Pair colNm = getNormal(surface[0], surface[1]);
						struct Pair colNorm[2];
						colNorm[1-objIndex] = colNm;
						colNorm[objIndex].x = colNm.x * -1;
						colNorm[objIndex].y = colNm.y * -1;
						
						double dti[2], m[2], I[2];
						struct Pair vti[2], nvti[2], preColVel[2]; 
						int k;
						
						for (k=0;k<2;k++) {
							vti[k].x = (colPos.x-objs[k]->pos.x);
							vti[k].y = (colPos.y-objs[k]->pos.y);
							dti[k] = getLength(vti[k]);
							
							nvti[k].x = vti[k].x/dti[k];
							nvti[k].y = vti[k].y/dti[k];
							preColVel[k].x = objs[k]->vel.x - objs[k]->angVel*vti[k].y;
							preColVel[k].y = objs[k]->vel.y + objs[k]->angVel*vti[k].x;
							
							m[k] = objs[k]->mass;
							I[k] = objs[k]->moi;
							
							
							
						}
						
						double preEnergy =DOT_PRODUCT(objs[0]->vel,objs[0]->vel)*m[0] + I[0]*(objs[0]->angVel*objs[0]->angVel) +
							DOT_PRODUCT(objs[1]->vel,objs[1]->vel)*m[1] + I[1]*(objs[1]->angVel*objs[1]->angVel);
							
						double impulse[k];
						double impulseDenom = (1/m[0] + 1/m[1])+((CROSS_PRODUCT(vti[0],colNorm[0]))*(CROSS_PRODUCT(vti[0],colNorm[0])))/(I[0]) + ((CROSS_PRODUCT(vti[1],colNorm[1]))*(CROSS_PRODUCT(vti[1],colNorm[1])))/(I[1]);
						struct Pair relColVel[2],pulse[2];
						double tpulse[2];
						for (k=0;k<2;k++) {
							
							relColVel[k].x = preColVel[k].x-preColVel[1-k].x;
							relColVel[k].y = preColVel[k].y-preColVel[1-k].y;
							impulse[k] = (-(2)*DOT_PRODUCT(relColVel[k],colNorm[1-objIndex]))/(impulseDenom);
							pulse[k].x = colNorm[k].x*impulse[k]/m[k];
							pulse[k].y = colNorm[k].y*impulse[k]/m[k];
							tpulse[k] = impulse[k] * CROSS_PRODUCT(vti[k],colNorm[k])/I[k];
							
							if (DOT_PRODUCT(vti[k],pulse[k])>0) {impulse[k]*=-1; pulse[k].x *= -1;pulse[k].y *= -1;tpulse[k] *= -1;}
						}
	
						struct Pair postVel[2];
						double postAngVel[2];
						for (k=0;k<2;k++) {
							postVel[k].x = objs[k]->vel.x + pulse[k].x;
							postVel[k].y = objs[k]->vel.y + pulse[k].y;
							postAngVel[k] = objs[k]->angVel + tpulse[k];
						}
						double postEnergy =DOT_PRODUCT(postVel[0],postVel[0])*m[0] + I[0]*(postAngVel[0]*postAngVel[0]) +
							DOT_PRODUCT(postVel[1],postVel[1])*m[1] + I[1]*(postAngVel[1]*postAngVel[1]);
							
						double gain = preEnergy/postEnergy; gain = (gain>1.0?1.0:gain);
						for (k=0;k<2;k++) {
							
							objs[k]->vel.x += pulse[k].x*gain;
							objs[k]->vel.y += pulse[k].y*gain;
							
							objs[k]->angVel += tpulse[k]*gain;
							
							objs[k]->pos.x += (dt-colDt)*objs[k]->vel.x;
							objs[k]->pos.y += (dt-colDt)*objs[k]->vel.y;
							objs[k]->angle += (dt-colDt)*objs[k]->angVel;
							
						}
						
						
						objs[0]->collisionHandler(objs[0],objs[1]);
						objs[1]->collisionHandler(objs[1],objs[0]);
						
					}
					other = other->next;	
				}	
			}
			cursor = cursor->next;
		}	
	}
	
	
		
	for (i=0;i<gameObjects->numBuckets;i++) {
		cursor = gameObjects->buckets[i].head;
		while(cursor!=NULL) {
			next = cursor->next;
			((GP)(cursor->obj))->simulate((GP)(cursor->obj),dt);
			cursor = next;
		}
	}
}

void drawScene() {
	static int frame = 0;
	frame++;
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glScaled(1.0/aspectRatio,1.0,1.0);
	
	struct GameObject* go;
	struct HashElement *cursor = NULL;
	int i;
	for (i=0;i<gameObjects->numBuckets;i++) {
		cursor = gameObjects->buckets[i].head;
		while(cursor!=NULL) {
			go = ((GP)(cursor->obj));
			if(go->allowDraw == 1) go->draw(go);
			cursor = cursor->next;
		}
	}
	SDL_GL_SwapBuffers();
}
