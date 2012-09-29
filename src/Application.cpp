#include <allegro.h>
#include <alfont.h>

#include <cstring>

#include "env.h"
#include "Application.h"
#include "SoundOGG.h"

Application * Application::theInstance = NULL;

Application::Application()
: KeepRunning(false)
, canvas(NULL)
, logFile(NULL)
, mouseButtons(NULL)
, prevMouseButtons(NULL)
, mousePressedLocs(NULL)
, prevMouseX(0)
, prevMouseY(0)
, prevMouseZ(0)
, loadingImg(NULL)
, font(NULL)
, lastGameTimeSecs(0)
{
   theInstance = this;
}

Application::~Application()
{
}

void Application::Execute()
{
   if (Init())
   {
      while (KeepRunning)
         Run();
   }
   
   Destroy();
}

void Application::log(const char* msg)
{
   if (ENABLE_LOGFILE && (logFile != NULL))
   {
      fprintf(logFile,msg);

      if (LOGFILE_AUTOFLUSH)
         fflush(logFile);
   }
}

Application& Application::GetApp()
{
   return *theInstance;
}

void Application::BroadcastEvent(int eventID)
{
   OnEvent(eventID);
}

void Application::HandleTiming()
{
   //
   // rest as long as necessary to maintain the constant desired framerate
   //
   
   double newGameTimeSecs = CLOCKS2SECS(clock());
   double deltaGameTimeSecs = newGameTimeSecs - lastGameTimeSecs;

   double desiredDeltaTimeSecs = 1.0 / (double)DESIRED_FPS;
   double extraTimeSecs = desiredDeltaTimeSecs - deltaGameTimeSecs;

   double restTimeSecs = (extraTimeSecs >= 0 ? extraTimeSecs : 0);
   int restTimeMS = (int)(restTimeSecs * 1000);
   rest(restTimeMS);

   newGameTimeSecs = CLOCKS2SECS(clock());
   deltaGameTimeSecs = newGameTimeSecs - lastGameTimeSecs;
   
   lastGameTimeSecs = newGameTimeSecs;
}

bool Application::Init()
{
   // logfile
   if (ENABLE_LOGFILE)
   {
      logFile = fopen(LOGFILE,"w");      
   }
   
   log("starting game\n");
   
   srand(time(NULL));
   
   // allegro
   log("initializing Allegro\n");
   
   if (allegro_init() != 0)
      return false;

   //
   // graphics
   //

   log("initializing graphics\n");
   
   set_color_depth(COLOR_DEPTH);

   int card;
   if (FULL_SCREEN)
      card = GFX_AUTODETECT_FULLSCREEN;
   else
      card = GFX_AUTODETECT_WINDOWED;

   if (set_gfx_mode(card,SCREEN_WIDTH,SCREEN_HEIGHT,0,0) != 0)
      return false;
   
   canvas = create_bitmap_ex(COLOR_DEPTH,screen->w,screen->h);
   if (canvas == NULL)
      return false;
   
   loadingImg = load_bitmap("data/loading.bmp",NULL);
   if (loadingImg == NULL)
      return false;
   blit(loadingImg,screen,0,0,0,0,screen->w,screen->h);

   //
   // font system
   //

   log("initializing font system\n");   
   if (alfont_init() != ALFONT_OK)
      return false;

   font = alfont_load_font(FONT_FILE);
   if (font == NULL)
      return false;   

   //
   // keyboard
   //
      
   log("initializing keyboard\n");

   if (install_keyboard() != 0)
      return false;

   memset(prevKeyState,0,256);

   //
   // mouse
   //
   
   log("initializing mouse\n");

   numMouseButtons = install_mouse();
   if (numMouseButtons < 0)
      return false;
   mouseButtons = new bool[numMouseButtons+1];
   prevMouseButtons = new bool[numMouseButtons+1];
   mousePressedLocs = new MousePos[numMouseButtons+1];

   for (int button = 0; button < numMouseButtons+1; button++)
   {
      mouseButtons[button] = false;
      prevMouseButtons[button] = false;
      mousePressedLocs[button].x = -1;
      mousePressedLocs[button].y = -1;
   }

   // timer
   log("initializing timer\n");
   
   if (install_timer() != 0)
      return false;

   //
   // sound
   //
   
   log("initializing base sound\n");

   if (install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL) != 0)
      return false;

   set_volume(ALLEGRO_SOUND_VOLUME,ALLEGRO_SOUND_VOLUME);

   // initialized successfully!
   log("initialization completed successfully\n");
   KeepRunning = true;   
   return true;
}

void Application::Destroy()
{
   log("destroying app resources\n");

   if (mouseButtons != NULL)
   {
      delete [] mouseButtons;
      mouseButtons = NULL;
   }
   
   if (prevMouseButtons != NULL)
   {
      delete [] prevMouseButtons;
      prevMouseButtons = NULL;
   }
   
   if (mousePressedLocs != NULL)
   {
      delete [] mousePressedLocs;
      mousePressedLocs = NULL;
   }

   if (loadingImg != NULL)
   {
      destroy_bitmap(loadingImg);
      loadingImg = NULL;
   }
   
   if (font != NULL)
   {
      alfont_destroy_font(font);
      font = NULL;
   }   
      
   // allegro shutdown   
   log("shutting down engine\n");
   allegro_exit();
   alfont_exit();
   
   log("game exiting\n");

   // logging  
   if (logFile != NULL)
   {
      fclose(logFile);
      logFile = NULL;
   }
}

void Application::Run()
{
   //
   // keyboard
   //
   
   if (keyboard_needs_poll())
      poll_keyboard();

   for (int k = 0; k < 256; k++)
   {
      if (key[k])
      {
         OnKeyPress(k);

         if (!prevKeyState[k])
            OnKeyPressed(k);
      }
      else if (!key[k])
      {
         if (prevKeyState[k])
            OnKeyReleased(k);
      }
   }

   memcpy(prevKeyState,(char*)key,256);

   //
   // mouse
   //
   
   if (mouse_needs_poll())
      poll_mouse();

   for (int button = 0; button < (numMouseButtons); button++)
   {
      if ((mouse_b & (1 << button)) != 0)
      {
         mouseButtons[button] = true;
      }
      else
      {
         mouseButtons[button] = false;
      }

      if (mouseButtons[button] && (!prevMouseButtons[button]))
      {
         OnMousePressed(button, mouse_x, mouse_y);

         mousePressedLocs[button].x = mouse_x;
         mousePressedLocs[button].y = mouse_y;
      }
      else if ((!mouseButtons[button]) && prevMouseButtons[button])
      {
         OnMouseReleased(button, mouse_x, mouse_y);

         if ((mousePressedLocs[button].x == mouse_x) &&
             (mousePressedLocs[button].y == mouse_y))
         {
            OnMouseClick(button,mouse_x,mouse_y);
         }
      }
   }

   memcpy(prevMouseButtons,mouseButtons,sizeof(bool)*(numMouseButtons+1));

   if ((mouse_x != prevMouseX) || (mouse_y != prevMouseY))
   {
      OnMouseMove(mouse_x,mouse_y);

      prevMouseX = mouse_x;
      prevMouseY = mouse_y;
   }

   // mouse wheel
   if (mouse_z > prevMouseZ) 
   {
	   OnMouseWheelUp( mouse_x, mouse_y );
	   prevMouseZ = mouse_z;
   }
   else
   if (mouse_z < prevMouseZ) 
   {
	   OnMouseWheelDown( mouse_x, mouse_y );
	   prevMouseZ = mouse_z;
   }

   //
   // run the game
   //
   
   show_mouse(NULL);
   RunGame();
   show_mouse(canvas);

   //
   // render canvas to the screen
   //
      
   blit(canvas,screen,0,0,0,0,screen->w,screen->h);

   //
   // sound polling (to ensure sounds currently playing will keep playing)
   //
   SoundOGG::PollSounds();

   //
   // handle timing
   //
   
   HandleTiming();
}
