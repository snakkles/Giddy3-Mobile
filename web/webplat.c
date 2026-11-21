/*
** Web (Emscripten) platform layer for Giddy 3.
**
** - Turns browser multi-touch (SDL finger events) into the per-finger
**   "mouse" events the webOS code expects (event.motion.which = finger slot).
** - Paces the game at 60 fps regardless of display refresh rate.
** - Persists saves/settings to IndexedDB.
*/
#include <emscripten.h>
#include <string.h>
#include "SDL.h"

#define WEB_W 640
#define WEB_H 480

static SDL_FingerID slot_finger[2];
static int slot_used[2];
static int touch_seen = 0;

void PDL_LaunchBrowser( const char *url )
{
  EM_ASM({ window.open(UTF8ToString($0), '_blank'); }, url);
}

void PDL_NotifyMusicPlaying( int playing ) { (void)playing; }

SDL_Surface *web_SetVideoMode( int w, int h, int bpp, Uint32 flags )
{
  (void)w; (void)h;
  return SDL_SetVideoMode( WEB_W, WEB_H, bpp, flags );
}

void web_SyncSaves( void )
{
  EM_ASM({ if (Module.syncSaves) Module.syncSaves(); });
}

/* Returns 1 if the event should be delivered to the game, 0 to drop it. */
static int translate( SDL_Event *ev )
{
  int i, slot = -1;
  SDL_TouchFingerEvent tf;

  switch( ev->type )
  {
  case SDL_FINGERDOWN:
  case SDL_FINGERMOTION:
  case SDL_FINGERUP:
    touch_seen = 1;
    tf = ev->tfinger;
    for( i=0; i<2; i++ )
      if( slot_used[i] && slot_finger[i] == tf.fingerId ) slot = i;

    if( tf.type == SDL_FINGERDOWN && slot == -1 )
    {
      for( i=0; i<2; i++ )
        if( !slot_used[i] ) { slot = i; break; }
      if( slot == -1 ) return 0;
      slot_used[slot] = 1;
      slot_finger[slot] = tf.fingerId;
    }
    if( slot == -1 ) return 0;

    memset( ev, 0, sizeof(*ev) );
    ev->type = (tf.type == SDL_FINGERDOWN) ? SDL_MOUSEBUTTONDOWN :
               (tf.type == SDL_FINGERUP)   ? SDL_MOUSEBUTTONUP : SDL_MOUSEMOTION;
    ev->motion.which = slot;
    ev->motion.x = (Sint32)(tf.x * WEB_W);
    ev->motion.y = (Sint32)(tf.y * WEB_H);
    if( tf.type == SDL_FINGERUP ) slot_used[slot] = 0;
    return 1;

  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP:
  case SDL_MOUSEMOTION:
    /* Browsers also emit emulated mouse events for touches; ignore those. */
    if( touch_seen ) return 0;
    if( ev->type == SDL_MOUSEMOTION && !(ev->motion.state & SDL_BUTTON(1)) ) return 0;
    {
      Sint32 x = (ev->type == SDL_MOUSEMOTION) ? ev->motion.x : ev->button.x;
      Sint32 y = (ev->type == SDL_MOUSEMOTION) ? ev->motion.y : ev->button.y;
      Uint32 t = ev->type;
      memset( ev, 0, sizeof(*ev) );
      ev->type = t;
      ev->motion.which = 0;
      ev->motion.x = x;
      ev->motion.y = y;
    }
    return 1;

  default:
    return 1;
  }
}

int web_PollEvent( SDL_Event *ev )
{
  while( SDL_PollEvent( ev ) )
    if( translate( ev ) ) return 1;
  return 0;
}

int web_WaitEvent( SDL_Event *ev )
{
  for( ;; )
  {
    if( web_PollEvent( ev ) ) return 1;
    emscripten_sleep( 16 );
  }
}

/* Wait (yielding to the browser) until the next 60 Hz game tick. */
EM_ASYNC_JS( void, wait_next_frame, (void), {
  var step = 1000 / 60;
  var now = performance.now();
  if (!Module.nextFrame || now - Module.nextFrame > 100) Module.nextFrame = now;
  Module.nextFrame += step;
  while (performance.now() < Module.nextFrame - 2) {
    await new Promise(function(r) {
      if (document.hidden) setTimeout(r, 50); else requestAnimationFrame(r);
    });
  }
});

void web_SwapBuffers( void )
{
  SDL_GL_SwapBuffers();
  wait_next_frame();
}

/* Called from JS when the page is hidden/closed, so progress isn't lost. */
extern void save_game( void );
EMSCRIPTEN_KEEPALIVE void web_on_hide( void )
{
  save_game();
}
