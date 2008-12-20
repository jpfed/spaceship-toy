#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "spacew.h"

void setup_opengl_screeninfo() {

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    /*SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);*/
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);	
	
}

/* This external variable is a load of crap.
Everyone has started to build around it, too.
We should ideally have a separate system for world coordinates and screen coordinates.
This file will need a few changes if we allow the user to select the resolution and/or color depth.
 */
float aspectRatio;

void setup_opengl_renderer(int width, int height) {
	
	aspectRatio = (float) width / (float) height;

    /* Our shading model--Gouraud (smooth). */
    glShadeModel(GL_SMOOTH );

    /* Culling. */
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );

    /* Set the clear color. */
    glClearColor( 0, 0, 0, 0 );
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
    /* Setup our viewport. */
    glViewport( 0, 0, width, height );

    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    gluPerspective( 60.0, aspectRatio, 1.0, 1024.0 );	
	
}

int main(int argc, char* argv[]) {

    
    int bpp = 0;
    int flags = 0;

    atexit(SDL_Quit);    
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) {
       printf("Unable to init SDL: %s\n", SDL_GetError());
	   exit(1);                                        
    }

    const SDL_VideoInfo* info = SDL_GetVideoInfo();
    if (!info) {printf("Video query failed: %s", SDL_GetError());
	exit(1);
	}

    int width = 800;
    int height = 600;
    bpp = info->vfmt->BitsPerPixel;

	setup_opengl_screeninfo();    

    flags = SDL_OPENGL | SDL_FULLSCREEN;
    
    SDL_Surface *screen;
    screen = SDL_SetVideoMode(width,height,bpp,SDL_OPENGL|SDL_FULLSCREEN);
    if (screen == NULL) {
	   printf("Unable to set video mode %d by %d: %s", width, height, SDL_GetError());
	   exit(1);
	}

	setup_opengl_renderer(width, height);
	
	objectInit();

	Uint8 *keys = malloc(256*sizeof(Uint8));
	int done =0; int i; int oversampleLimit = 1;
	while (!done) {
		SDL_PumpEvents();
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_ESCAPE]) {exit(1);}
		handleInput(keys);
		for (i=0;i<oversampleLimit;i++)
			simulateWorld();
		
		drawScene();
	}	  		  	  

    return 0;
}
