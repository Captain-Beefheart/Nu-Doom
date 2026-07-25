//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"
#include "crispy.h"
#include "doomstat.h"   // gamestate / paused for mouse-grab decisions

#include <stdio.h>
#include <unistd.h>

#include <stdbool.h>
#include <SDL.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

// Mouse: relative (grabbed) mode is used so the pointer is captured to the
// window and we get raw motion deltas to drive turning / mouselook.
static int s_MouseGrabbed = 0;
static int s_WindowFocused = 1;

static unsigned char convertToDoomKey(unsigned int key){
  switch (key)
    {
    case SDLK_RETURN:
      key = KEY_ENTER;
      break;
    case SDLK_ESCAPE:
      key = KEY_ESCAPE;
      break;
    case SDLK_LEFT:
      key = KEY_LEFTARROW;
      break;
    case SDLK_RIGHT:
      key = KEY_RIGHTARROW;
      break;
    case SDLK_UP:
      key = KEY_UPARROW;
      break;
    case SDLK_DOWN:
      key = KEY_DOWNARROW;
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      key = KEY_FIRE;
      break;
    case SDLK_SPACE:
      key = KEY_USE;
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      key = KEY_RSHIFT;
      break;
    case SDLK_LALT:
    case SDLK_RALT:
      key = KEY_LALT;
      break;
    case SDLK_F2:
      key = KEY_F2;
      break;
    case SDLK_F3:
      key = KEY_F3;
      break;
    case SDLK_F4:
      key = KEY_F4;
      break;
    case SDLK_F5:
      key = KEY_F5;
      break;
    case SDLK_F6:
      key = KEY_F6;
      break;
    case SDLK_F7:
      key = KEY_F7;
      break;
    case SDLK_F8:
      key = KEY_F8;
      break;
    case SDLK_F9:
      key = KEY_F9;
      break;
    case SDLK_F10:
      key = KEY_F10;
      break;
    case SDLK_F11:
      key = KEY_F11;
      break;
    case SDLK_EQUALS:
    case SDLK_PLUS:
      key = KEY_EQUALS;
      break;
    case SDLK_MINUS:
      key = KEY_MINUS;
      break;
    default:
      key = tolower(key);
      break;
    }

  return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode){
  unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}
// The mouse is grabbed only while actually playing a level with no menu open,
// so the pointer is free at the title screen, in menus, and while paused.
static void UpdateMouseGrab(void)
{
  extern boolean menuactive;   // m_menu.c
  int want = s_WindowFocused && !menuactive && !paused
             && gamestate == GS_LEVEL;

  if (want != s_MouseGrabbed)
  {
    SDL_SetRelativeMouseMode(want ? SDL_TRUE : SDL_FALSE);
    SDL_GetRelativeMouseState(NULL, NULL);   // flush any pending delta
    s_MouseGrabbed = want;
  }
}

static void handleKeyInput(){
  SDL_Event e;
  while (SDL_PollEvent(&e)){
    if (e.type == SDL_QUIT){
      puts("Quit requested");
      atexit(SDL_Quit);
      exit(1);
    }
    if (e.type == SDL_KEYDOWN) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyPress:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(1, e.key.keysym.sym);
    } else if (e.type == SDL_KEYUP) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyRelease:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(0, e.key.keysym.sym);
    } else if (e.type == SDL_WINDOWEVENT) {
      if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        s_WindowFocused = 1;
      else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        s_WindowFocused = 0;
    }
  }

  UpdateMouseGrab();
}

// Read the accumulated mouse motion and button state for this frame. Returns
// 1 when the mouse is grabbed (values valid), 0 otherwise (all zeroed). Doom
// button bits: 0=left(fire), 1=right, 2=middle.
int DG_GetMouse(int* buttons, int* dx, int* dy)
{
  int x, y;
  Uint32 state = SDL_GetRelativeMouseState(&x, &y);   // also flushes the delta

  if (!s_MouseGrabbed)
  {
    *buttons = 0; *dx = 0; *dy = 0;
    return 0;
  }

  *buttons = 0;
  if (state & SDL_BUTTON(SDL_BUTTON_LEFT))   *buttons |= 1;
  if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))  *buttons |= 2;
  if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) *buttons |= 4;
  *dx = x;
  *dy = y;
  return 1;
}


void DG_Init(){
  window = SDL_CreateWindow("DOOM",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            DOOMGENERIC_RESX,
                            DOOMGENERIC_RESY,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                            );

  // Setup renderer
  renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
  // Clear winow
  SDL_RenderClear( renderer );
  // Render the rect to the screen
  SDL_RenderPresent(renderer);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, DOOMGENERIC_RESX, DOOMGENERIC_RESY);

  DG_SetVSync(crispy.vsync);   // apply the saved Crispness VSync setting
}

// Crispness: toggle vertical sync on the live renderer.
void DG_SetVSync(int enabled)
{
  if (renderer != NULL)
    SDL_RenderSetVSync(renderer, enabled ? 1 : 0);
}

void DG_DrawFrame()
{
  SDL_UpdateTexture(texture, NULL, DG_ScreenBuffer, DOOMGENERIC_RESX*sizeof(uint32_t));

  // Crispness: smooth (linear) vs crisp (nearest) scaling when the window is
  // resized away from the native resolution.
  SDL_SetTextureScaleMode(texture,
      crispy.smoothscaling ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  // Crispness: cap the render framerate. Game logic stays at 35Hz (paced by
  // I_GetTime), so this only throttles how often we present frames.
  if (crispy.fpslimit > 0)
  {
    static uint32_t lastframe = 0;
    uint32_t target = 1000u / (uint32_t) crispy.fpslimit;
    uint32_t now = SDL_GetTicks();
    uint32_t elapsed = now - lastframe;
    if (elapsed < target)
      SDL_Delay(target - elapsed);
    lastframe = SDL_GetTicks();
  }

  handleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
  SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
  return SDL_GetTicks();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}

void DG_SetWindowTitle(const char * title)
{
  if (window != NULL){
    SDL_SetWindowTitle(window, title);
  }
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }
    

    return 0;
}