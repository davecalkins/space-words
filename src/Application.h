#pragma once

#include <cstdio>

struct BITMAP;
struct ALFONT_FONT;

class Application
{
public:
   Application();
   virtual ~Application();
       
   void Execute();

   static Application& GetApp();

   void BroadcastEvent(int eventID);
   
private:
   static Application * theInstance;
   
   void Run();

   FILE* logFile;

   char prevKeyState[256];

   int numMouseButtons;
          
   bool * mouseButtons;
   bool * prevMouseButtons;

   struct MousePos
   {
      int x;
      int y;
   };
   MousePos * mousePressedLocs;

   int prevMouseX;
   int prevMouseY;
   int prevMouseZ;
   
   BITMAP * loadingImg;
   
   double lastGameTimeSecs;

   void HandleTiming();
       
protected:
   virtual bool Init();
   virtual void Destroy();
   
   bool KeepRunning;
   
   BITMAP * canvas;
   ALFONT_FONT * font;
   
   void log(const char* msg);
   
   virtual void RunGame() = 0;
   virtual void OnKeyPress(int keyCode) = 0;
   virtual void OnKeyPressed(int keyCode) = 0;
   virtual void OnKeyReleased(int keyCode) = 0;
   virtual void OnMouseMove(int x, int y) = 0;
   virtual void OnMouseClick(int button, int x, int y) = 0;
   virtual void OnMousePressed(int button, int x, int y) = 0;
   virtual void OnMouseReleased(int button, int x, int y) = 0;
   virtual void OnMouseWheelUp(int x, int y) = 0;
   virtual void OnMouseWheelDown(int x, int y) = 0;
   virtual void OnEvent(int eventID) = 0;
};
