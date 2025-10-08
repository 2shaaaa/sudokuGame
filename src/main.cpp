#include <iostream>
#include <stdio.h>
#include <SDL_image.h>
#include <string>
#include <SDL.h>
using namespace std;

const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

enum KeyPressSurfaces {
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

SDL_Surface* gKeyPressSurfaces[ KEY_PRESS_SURFACE_TOTAL ];

SDL_Surface* gCurrentSurface = NULL;

SDL_Window* gWindow = NULL;

SDL_Surface* gScreenSurface = NULL;

SDL_Surface* loadSurface(string path) {
	SDL_Surface* optimizedSurface = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL) {
		cerr << "Unable to load image " << path << "! SDL Error: " << SDL_GetError() << endl;
	} else {
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL) {
			cerr << "Unable to optimize image " << path << "! SDL Error: " << SDL_GetError() << endl;
		}
		SDL_FreeSurface(loadedSurface);
	}
	return optimizedSurface;
};

// Start up SDL and create window
bool init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return false;
    } else {
        gWindow = SDL_CreateWindow("Hello World 101", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            success = false;
        } else {
            int imgFlags = IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags)) {
				cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
				success = false;
			} else {
				gScreenSurface = SDL_GetWindowSurface(gWindow);
       		 }
    	}
    return success;
	}
};

// Load media
bool loadMedia() {
    bool success = true;

	gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = loadSurface("image/press.bmp");
	if( gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] == NULL) {
		cerr << "Failed to load default image! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] = loadSurface("image/up.bmp");
	if( gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] == NULL) {
		cerr << "Fail to load up imange! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = loadSurface("image/down.bmp");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] == NULL) {
		cerr << "Fail to load down imange! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = loadSurface("image/left.bmp");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] == NULL) {
		cerr << "Fail to load left imange! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}
	gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = loadSurface("image/right.bmp");
	if (gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] == NULL) {
		cerr << "Fail to load right imange! SDL Error: " << SDL_GetError() << endl;
		success = false;
	}		
		return success;
};

// Free media and shut down SDL
void close() {
	for( int i = 0; i < KEY_PRESS_SURFACE_TOTAL; ++i )
	{
		SDL_FreeSurface( gKeyPressSurfaces[ i ] );
		gKeyPressSurfaces[ i ] = NULL;
	}
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	SDL_Quit();
};


int main(int argc, char* args[])
{
    if (!init()) {
        cerr << "Failed to initialize!" << endl;
    } else {
        if (!loadMedia()) {
            cerr << "Failed to load media!" << endl;
        } else {		
			bool quit = false;
			SDL_Event e;
			gCurrentSurface = loadSurface("image/mainBackground.png");
			SDL_FillRect( gScreenSurface, NULL, SDL_MapRGB( gScreenSurface->format, 0xFF, 0xFF, 0xFF ) );
			while( !quit )
			{
				while( SDL_PollEvent( &e ) != 0 )
				{
					if( e.type == SDL_QUIT )
					{
						quit = true;
					} else if (e.type == SDL_KEYDOWN) {
						switch(e.key.keysym.sym)
						{
							case SDLK_ESCAPE:
							SDL_FillRect( gScreenSurface, NULL, SDL_MapRGB( gScreenSurface->format, 0xFF, 0xFF, 0xFF ) );
							gCurrentSurface = loadSurface("image/mainBackground.png");
							break;

							case SDLK_w:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
							break;
							case SDLK_UP:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
							break;

							case SDLK_s:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
							break;
							case SDLK_DOWN:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
							break;

							case SDLK_a:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
							break;
							case SDLK_LEFT:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
							break;

							case SDLK_d:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
							break;
							case SDLK_RIGHT:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
							break;

							default:
							gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
							break;
						}
					}
				}
				//Apply the current image
				SDL_Rect stretchRect;
				stretchRect.x = 0;
				stretchRect.y = 0;
				stretchRect.w = SCREEN_WIDTH;
				stretchRect.h = SCREEN_HEIGHT;
				SDL_BlitScaled( gCurrentSurface, NULL, gScreenSurface, &stretchRect );
				//Update the surface
				SDL_UpdateWindowSurface(gWindow);
			 }
        }
    } close();
    return 0;
}