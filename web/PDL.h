/*
** Web (Emscripten) stand-in for the webOS PDL header.
** Included by the game sources after SDL.h, so it also reroutes a few
** SDL calls through the web platform layer in webplat.c.
*/
#ifndef GIDDY3_WEB_PDL_H
#define GIDDY3_WEB_PDL_H

#include "SDL.h"

#define PDL_TRUE  1
#define PDL_FALSE 0

void PDL_LaunchBrowser( const char *url );
void PDL_NotifyMusicPlaying( int playing );

/* Platform hooks (webplat.c) */
int          web_PollEvent( SDL_Event *ev );
int          web_WaitEvent( SDL_Event *ev );
void         web_SwapBuffers( void );
SDL_Surface *web_SetVideoMode( int w, int h, int bpp, Uint32 flags );
void         web_SyncSaves( void );
int          web_OpenAudio( void (*cb)( void *, Sint8 *, int ), int freq );
void         web_PauseAudio( int pause );

#define SDL_PollEvent     web_PollEvent
#define SDL_WaitEvent     web_WaitEvent
#define SDL_GL_SwapBuffers web_SwapBuffers
#define SDL_SetVideoMode  web_SetVideoMode
#define SDL_PauseAudio    web_PauseAudio

#endif
