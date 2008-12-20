#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "coreutils.h"
#include "universe.h"
#include "spacew.h"
#include "ship.h"
#include "bullet.h"
#include "particles.h"

#define SHIP_SIZE 0.025
#define SHIP_TEMP_INVINCE 20

/* Abstract away the controls.  Passing around key maps is silly.*/
extern float aspectRatio;

void shipDraw(struct GameObject *go) {
	if (go->type == SHIP) {
		struct Ship *ship = (struct Ship *)(go->objData);
		glPushMatrix();
		glTranslated(go->pos.x, go->pos.y,0);
		glRotated(go->angle,0,0,1);
		glColor3d(1.0-ship->tempInvince%2,0.0,0.0);
		glBegin(GL_POLYGON);
		glVertex2d(SHIP_SIZE,0);
		glVertex2d(-SHIP_SIZE/2,SHIP_SIZE/2);
		glVertex2d(-SHIP_SIZE/2,-SHIP_SIZE/2);
		glEnd();
		if (ship->thrusters) {
			glColor3d(1,1,1);
			glBegin(GL_POLYGON);
			glVertex2d(-SHIP_SIZE/2,SHIP_SIZE/4);
			glVertex2d(-SHIP_SIZE,0.0);
			glVertex2d(-SHIP_SIZE/2,-SHIP_SIZE/4);
			glEnd();	
		}
		if (ship->retro) {
			glColor3d(1,1,1);
			glBegin(GL_POLYGON);
			glVertex2d(SHIP_SIZE,SHIP_SIZE/8);
			glVertex2d(SHIP_SIZE,-SHIP_SIZE/8);
			glVertex2d(SHIP_SIZE*1.5,0.0);
			glEnd();
		}
		if (ship->turnRight) {
			glColor3d(1,1,1);
			glBegin(GL_POLYGON);
			glVertex2d(SHIP_SIZE/2,SHIP_SIZE/6);
			glVertex2d(SHIP_SIZE/4,SHIP_SIZE*2/3);
			glVertex2d(0,SHIP_SIZE/3);
			glEnd();
		}
		if (ship->turnLeft) {
			glColor3d(1,1,1);
			glBegin(GL_POLYGON);
			glVertex2d(SHIP_SIZE/2,-SHIP_SIZE/6);
			glVertex2d(0,-SHIP_SIZE/3);
			glVertex2d(SHIP_SIZE/4,-SHIP_SIZE*2/3);
			glEnd();
		}
		
		glPopMatrix();
		
		glBegin(GL_LINES);
		glColor3d(1,1,1);
		
		glVertex2d(-0.95*aspectRatio,-0.90);
		glVertex2d(-0.95*aspectRatio+ship->heat/20.0,-0.90);
		
		glVertex2d(-0.95*aspectRatio,-0.95);
		glVertex2d(-0.95*aspectRatio+ship->shield/20,-0.95);
		
		glEnd();
		
	}
}
void shipSimulate(struct GameObject *go, double dt) {
	if (go->type == SHIP) {
		struct Ship *ship = (struct Ship *)(go->objData);
		double vel = sqrt((go->vel.x)*(go->vel.x)+(go->vel.y)*(go->vel.y));
		if (vel > ship->warpLimit) {
			go->vel.x*=ship->warpLimit/vel;
			go->vel.y*=ship->warpLimit/vel;
		}
		if (ship->firingBullet != NULL) {
			go->force.x	+= (go->vel.x-ship->firingBullet->vel.x)*ship->firingBullet->mass/dt;
			go->force.y	+= (go->vel.y-ship->firingBullet->vel.y)*ship->firingBullet->mass/dt;
			ship->firingBullet=NULL;
		}
		if (!ship->tempInvince)
			go->torque += (ship->targetAngVel - go->angVel) * go->moi/dt;
		
		
		
		ship->engines->angle = go->angle;
		ship->rengines->angle = 180+go->angle;
		ship->engines->pos = go->pos;
		ship->engines->pos.x-=cos(CONVERT_TO_RAD*go->angle)*SHIP_SIZE/2;
		ship->engines->pos.y-=sin(CONVERT_TO_RAD*go->angle)*SHIP_SIZE/2;
		ship->rengines->pos = go->pos;
		ship->rengines->pos.x+=cos(CONVERT_TO_RAD*go->angle)*SHIP_SIZE;
		ship->rengines->pos.y+=sin(CONVERT_TO_RAD*go->angle)*SHIP_SIZE;
		ship->engines->vel = go->vel;
		ship->rengines->vel = go->vel;
		
		ship->tractorBeam->pos = go->pos;
		struct ParticleSystem *tps=(struct ParticleSystem *)(ship->tractorBeam->objData);
		if (ship->tractoredObject!=NULL) {
			struct Pair vect, nvect, fvect;
			
			double dist;
			vect.x = ship->tractoredObject->pos.x-go->pos.x;
			vect.y = ship->tractoredObject->pos.y-go->pos.y;
			dist = getLength(vect);
			if (dist<1.0) {
			ship->tractorBeam->angle = 180+atan2(vect.y,vect.x)/(CONVERT_TO_RAD);
			tps->overallFade = 1.0;
			tps->intensity = 0.5;
			
			tps->speed = 4*dist;
			nvect.x = vect.x/dist;
			nvect.y = vect.y/dist;
			fvect.x = 10*(ship->tractorLength-dist)*nvect.x;
			fvect.y = 10*(ship->tractorLength-dist)*nvect.y;
			ship->tractoredObject->force.x+=fvect.x;
			ship->tractoredObject->force.y+=fvect.y;
			go->force.x-=fvect.x;
			go->force.y-=fvect.y;
			}
			else {
				ship->tractoredObject = NULL;
				tps->overallFade = 0.75;
			}
		}
		else {
			tps->overallFade = 0.75;
		}
		
		applyMomentum(go,dt);
		if (toroidalWrap(go,aspectRatio*2,2)) {ship->tractoredObject = NULL;}
		applyCooldown(&(ship->heat));
		applyCooldown(&(ship->tempInvince));
		
	}
}
void shipInput(struct GameObject *go, Uint8 *keys) {
	if (go->type == SHIP) {
		struct Ship *ship = (struct Ship *)(go->objData);
		struct ParticleSystem *eps=(struct ParticleSystem *)(ship->engines->objData);
		struct ParticleSystem *rps=(struct ParticleSystem *)(ship->rengines->objData);
		struct Pair targVel;
		double dotProduct, worstDotProduct, bestDotProduct, fthrust, rthrust;
		bestDotProduct = (ship->warpLimit)*(ship->warpLimit);
		worstDotProduct = -bestDotProduct;
		targVel.x = (ship->warpLimit)*cos(CONVERT_TO_RAD*go->angle);
		targVel.y = (ship->warpLimit)*sin(CONVERT_TO_RAD*go->angle);
		dotProduct = targVel.x*go->vel.x + targVel.y*go->vel.y;
		rthrust = (dotProduct-worstDotProduct)/(bestDotProduct-worstDotProduct);
		fthrust = 1-rthrust;
		if (keys[SDLK_UP]) {
			eps->overallFade = 1.0;
			eps->intensity=0.9*pow( fthrust, 0.5 );
			go->force.x+=fthrust*(ship->speed)*cos(CONVERT_TO_RAD*go->angle)*go->mass;
			go->force.y+=fthrust*(ship->speed)*sin(CONVERT_TO_RAD*go->angle)*go->mass;
			ship->thrusters = (fthrust > 0.125);
		} else {
			eps->overallFade=0.75;
			ship->thrusters=0;
		}
		
		if (keys[SDLK_DOWN]) {
			rps->overallFade = 1.0;
			rps->intensity=0.9*pow( rthrust, 0.5 );
			go->force.x-=rthrust*(ship->speed)*cos(CONVERT_TO_RAD*go->angle)*go->mass;
			go->force.y-=rthrust*(ship->speed)*sin(CONVERT_TO_RAD*go->angle)*go->mass;
			ship->retro = (rthrust > 0.125);
		} else {
			rps->overallFade=0.75;
			ship->retro = 0;
		}
		ship->turnLeft = 0;
		ship->turnRight = 0;
		ship->targetAngVel = 0.0;
		if (!ship->tempInvince) {
			if (keys[SDLK_LEFT]) 
				ship->targetAngVel = ship->turnSpeed;
			if (keys[SDLK_RIGHT])
				ship->targetAngVel = -ship->turnSpeed;
			if (ship->targetAngVel - go->angVel > 0.25)
				ship->turnLeft = 1;
			if (ship->targetAngVel - go->angVel < -0.25)
				ship->turnRight = 1;
		}
		 
		if (keys[SDLK_SPACE]) {
			if (ship->heat == 0) {
				struct GameObject *bo = initBullet();
				struct Bullet *bullet = (struct Bullet *)bo->objData;
				double firingXVel = (bullet->speed)*cos(CONVERT_TO_RAD*go->angle);
				double firingYVel = (bullet->speed)*sin(CONVERT_TO_RAD*go->angle);

				bo->vel.x = go->vel.x + firingXVel;
				bo->vel.y = go->vel.y + firingYVel;
			
				bo->pos.x = go->pos.x + SHIP_SIZE*cos(CONVERT_TO_RAD*go->angle);
				bo->pos.y = go->pos.y + SHIP_SIZE*sin(CONVERT_TO_RAD*go->angle);
				
				ship->firingBullet = bo;
				
				ship->heat+=5;
				hashAdd(gameObjects,(void*)bo);
			}
		}
		
		if (keys[SDLK_v]) {
			if (!ship->tractor) {
				struct GameObject *other, *bestOther;
				bestOther = NULL;
				struct HashElement *cursor = NULL;
				int i;
				struct Pair vect;
				double dist, bestDist;
				bestDist = RAND_MAX;
				for (i=0;i<gameObjects->numBuckets;i++) {
					cursor = gameObjects->buckets[i].head;
					while(cursor!=NULL) {
						other = ((GP)(cursor->obj));
						if ((other->type != BULLET) && other->phased<=0) {
							vect.x = other->pos.x - go->pos.x;
							vect.y = other->pos.y - go->pos.y;
							dist = getLength(vect);
							if (dist<bestDist && dist > 2*SHIP_SIZE) {
								bestDist = dist;
								bestOther = other;	
							}
						}
						cursor = cursor->next;
					}
				}
				ship->tractorLength = bestDist;
				ship->tractoredObject = bestOther;
				ship->tractor = bestOther!=NULL;
			}
		}
		else {
			ship->tractor = 0;
			ship->tractoredObject = NULL;
		}
		
	}
}

void shipCollide(struct GameObject* go, struct GameObject* other) {
	struct Ship *ship = (struct Ship*)(go->objData);
	if (!ship->tempInvince) {
		ship->heat+=3;
		ship->tempInvince = SHIP_TEMP_INVINCE;
	}
}

struct GameObject *initShip() {
	struct GameObject *go = gameObjectInit();
	struct Ship *ship = malloc(sizeof(struct Ship));
	ship->weapon = 10.0;
	ship->shield = 10.0;
	ship->speed = 3;
	ship->warpLimit = 6;
	ship->turnSpeed = 500;
	ship->heat = 0;
	ship->tempInvince = 0;
	ship->thrusters = 0;
	ship->retro = 0;
	ship->turnLeft = 0;
	ship->turnRight = 0;
	ship->targetAngVel = 0.0;
	ship->tractor = 0;
	ship->tractorLength = 0.0;
	ship->tractoredObject = NULL;
	
	ship->engines = initParticles(go);
	ship->rengines = initParticles(go);
	ship->tractorBeam = initParticles(go);
	
	hashAdd(gameObjects,ship->engines);
	hashAdd(gameObjects,ship->rengines);
	hashAdd(gameObjects,ship->tractorBeam);
	struct ParticleSystem *eps = (struct ParticleSystem *)(ship->engines->objData);
	struct ParticleSystem *rps = (struct ParticleSystem *)(ship->rengines->objData);
	struct ParticleSystem *tps = (struct ParticleSystem *)(ship->tractorBeam->objData);
	eps->intensity = rps->intensity = tps->intensity = 0.0;
	eps->overallFade = rps->overallFade = tps->overallFade = 1.0;
	eps->rMax = rps->rMax = 1.0; tps->rMax = 0.5;
	eps->rMin = rps->rMin = 0.0; tps->rMin = 0.0;
	eps->gMax = rps->gMax = 0.5; tps->gMax = 0.5;
	eps->gMin = rps->gMin = 0.0; tps->gMin = 0.0;
	eps->bMax = rps->bMax = 0.0; tps->bMax = 1.0;
	eps->bMin = rps->bMin = 0.0; tps->bMin = 1.0;
	tps->inward = 1;
	tps->speedDev = tps->speed/2;
	tps->angleDev = 10;
	go->mass = GO_DENSITY*SHIP_SIZE*SHIP_SIZE;
	go->moi = SHIP_SIZE*SHIP_SIZE*go->mass/320;
	go->maxDim = SHIP_SIZE;
	
	struct Polygon *poly = malloc(sizeof(struct Polygon));
	poly->numPoints = 3;
	poly->points = malloc(3*sizeof(struct Pair));
	poly->points[0].x = SHIP_SIZE; poly->points[0].y = 0;
	poly->points[1].x = -SHIP_SIZE/2; poly->points[1].y = SHIP_SIZE/2;
	poly->points[2].x = -SHIP_SIZE/2; poly->points[2].y = -SHIP_SIZE/2;
	go->bounds = poly;
	
	go->objData = (void*)ship;
	go->type = SHIP;
	go->draw = shipDraw;
	go->simulate = shipSimulate;
	go->inputHandler = shipInput;
	go->collisionHandler = shipCollide;
	return go;
}

