#pragma once

#include <string>

struct BITMAP;

/**
 * re-usable button class
 */
class Button
{
public:
   Button(std::string imgFileNormal, std::string imgFileMouseOver, std::string imgFileDisabled,
      int xPos, int yPos, int mouseOverEvent, int clickEvent, bool enabled = true, bool visible = true);
   virtual ~Button();

   void setEnabled(bool enabled);
   bool getEnabled();

   void setVisible(bool visible);
   bool getVisible();

   int GetWidth();
   int GetHeight();

   int GetX();
   int GetY();

   void SetX(int x);
   void SetY(int y);

   bool Init();
   void Destroy();

   void Run(BITMAP * canvas);
   void OnMouseMove(int x, int y);
   void OnMouseReleased(int button, int x, int y);

private:
   bool initialized;

   std::string imgFileNormal, imgFileMouseOver, imgFileDisabled;
   int xPos, yPos;
   int mouseOverEvent, clickEvent;
   bool enabled, visible;

   BITMAP * imgNormal;
   BITMAP * imgMouseOver;
   BITMAP * imgDisabled;

   int lastMouseX, lastMouseY;

   bool PtInBtn(int x, int y);
};
