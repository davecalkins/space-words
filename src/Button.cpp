#include <allegro.h>

#include "Button.h"
#include "Application.h"

Button::Button(std::string imgFileNormal, std::string imgFileMouseOver, std::string imgFileDisabled,
   int xPos, int yPos, int mouseOverEvent, int clickEvent,
   bool enabled /*= true*/, bool visible /*= true*/)
: initialized(false)
, imgFileNormal(imgFileNormal)
, imgFileMouseOver(imgFileMouseOver)
, imgFileDisabled(imgFileDisabled)
, xPos(xPos)
, yPos(yPos)
, mouseOverEvent(mouseOverEvent)
, clickEvent(clickEvent)
, enabled(enabled)
, visible(visible)
, imgNormal(NULL)
, imgMouseOver(NULL)
, imgDisabled(NULL)
, lastMouseX(0)
, lastMouseY(0)
{
}

Button::~Button()
{
   Destroy();
}

void Button::setEnabled(bool enabled)
{
   this->enabled = enabled;
}

bool Button::getEnabled()
{
   return enabled;
}

void Button::setVisible(bool visible)
{
   this->visible = visible;
}

bool Button::getVisible()
{
   return visible;
}

int Button::GetWidth()
{
   return imgNormal->w;
}

int Button::GetHeight()
{
   return imgNormal->h;
}

int Button::GetX()
{
   return xPos;
}

int Button::GetY()
{
   return yPos;
}

void Button::SetX(int x)
{
   xPos = x;
}

void Button::SetY(int y)
{
   yPos = y;
}

bool Button::Init()
{
   Destroy();

   imgNormal = load_bitmap(imgFileNormal.c_str(),NULL);
   if (imgNormal == NULL)
      return false;

   imgMouseOver = load_bitmap(imgFileMouseOver.c_str(),NULL);
   imgDisabled = load_bitmap(imgFileDisabled.c_str(),NULL);

   initialized = true;

   return true;
}

void Button::Destroy()
{
   if (imgNormal != NULL)
   {
      destroy_bitmap(imgNormal);
      imgNormal = NULL;
   }

   if (imgMouseOver != NULL)
   {
      destroy_bitmap(imgMouseOver);
      imgMouseOver = NULL;
   }

   if (imgDisabled != NULL)
   {
      destroy_bitmap(imgDisabled);
      imgDisabled = NULL;
   }

   initialized = false;
}

void Button::Run(BITMAP * canvas)
{
   if (!initialized)
      return;

   if (!visible)
      return;

   BITMAP * imgToDraw = imgNormal;

   bool mouseIsOverButton = PtInBtn(lastMouseX,lastMouseY);

   if (!enabled && (imgDisabled != NULL))
      imgToDraw = imgDisabled;
   else if (enabled && mouseIsOverButton && (imgMouseOver != NULL))
      imgToDraw = imgMouseOver;

   blit(imgToDraw,canvas,0,0,xPos,yPos,imgToDraw->w,imgToDraw->h);

   if (mouseIsOverButton)
   {
      Application::GetApp().BroadcastEvent(mouseOverEvent);
   }
}

void Button::OnMouseMove(int x, int y)
{
   lastMouseX = x;
   lastMouseY = y;
}

void Button::OnMouseReleased(int button, int x, int y)
{
   if (!initialized)
      return;

   if (!visible)
      return;

   if (!enabled)
      return;

   if (!PtInBtn(x,y))
      return;

   Application::GetApp().BroadcastEvent(clickEvent);
}

bool Button::PtInBtn(int x, int y)
{
   if ((x >= xPos) &&
       (x < (xPos + imgNormal->w)) &&
       (y >= yPos) &&
       (y < (yPos + imgNormal->h)))
   {
      return true;
   }

   return false;
}

