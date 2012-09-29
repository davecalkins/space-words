#include "Application.h"

struct BITMAP;
class Button;
class SoundOGG;
class sprite;

#include <ctime>
#include <string>
#include <map>
#include <vector>

#define NUM_WORD_BTNS 6

#define WORD_VOLUME 255

#define STARFIELD_LEFTEDGE 25
#define STARFIELD_TOPEDGE  25
#define STARFIELD_WIDTH    733
#define STARFIELD_HEIGHT   717

#define PATROL_DURATION_SECS 3
#define PREWORD_DELAY_SECS 1

#define ENEMYSHIP_SPEED 3

#define NUM_DISPLAYED_WORDS_AT_ONCE 6

#define STARFIELDB_NUMSTARS 200
#define STARFIELDB_COLOR1   makecol(155,155,155)
#define STARFIELDB_COLOR2   makecol(55,55,55)
#define SCROLLB_SPEED       2

#define STARFIELD_NUMSTARS 100
#define STARFIELD_COLOR1   makecol(255,255,255)
#define STARFIELD_COLOR2   makecol(155,155,155)
#define STARFIELD_COLOR3   makecol(55,55,55)
#define SCROLL_SPEED       4

#define ALERTLIGHT_WIDTH       198
#define ALERTLIGHT_HEIGHT      60
#define ALERTLIGHTL_X          47
#define ALERTLIGHTL_Y          25
#define ALERTLIGHTR_X          539
#define ALERTLIGHTR_Y          25
#define ALERTLIGHT_TOTALFRAMES 4
#define ALERTLIGHT_FRAMEDELAY  10

#define EXPLOSION_DELAY_SECS 2

#define NUM_EXPLOSION_SPRITES 80
#define EXPLOSION_FRAME_WIDTH 65
#define EXPLOSION_FRAME_HEIGHT 65
#define EXPLOSION_NUM_FRAMES 7
#define EXPLOSION_FRAME_DELAY 5

#define PHOTON_FRAME_WIDTH 65
#define PHOTON_FRAME_HEIGHT 65
#define PHOTON_NUM_FRAMES 7
#define PHOTON_FRAME_DELAY 15
#define PHOTON_SPEED 2

#define SCORE_X 790
#define SCORE_Y 710
#define SCORE_HEIGHT 40
#define SCORE_COLOR makecol(255,255,255)

#define PTS_PER_LETTER 1

#define TIMELEFT_X 794
#define TIMELEFT_Y 664
#define TIMELEFT_WIDTH 200
#define TIMELEFT_HEIGHT 45
#define TIMELEFT_BORDER_COLOR makecol(255,255,255)
#define TIMELEFT_FILL_COLOR makecol(0,200,0)

#define TOTAL_TIME_ALLOWED 600

class Game : public Application
{
public:
   Game();
   virtual ~Game();
   
protected:
   virtual bool Init();
   virtual void Destroy();

   virtual void RunGame();   
   virtual void OnKeyPress(int keyCode);
   virtual void OnKeyPressed(int keyCode);
   virtual void OnKeyReleased(int keyCode);
   virtual void OnMouseMove(int x, int y);
   virtual void OnMouseClick(int button, int x, int y);
   virtual void OnMousePressed(int button, int x, int y);
   virtual void OnMouseReleased(int button, int x, int y);
   virtual void OnMouseWheelUp(int x, int y);
   virtual void OnMouseWheelDown(int x, int y);
   virtual void OnEvent(int eventID);

private:
   BITMAP * backgroundImg;
   BITMAP * starfieldBImg;
   BITMAP * starfieldImg;
   int yBPos, yPos;

   class Word
   {
   public:
      Word();
      virtual ~Word();

      std::string text;
      SoundOGG * sound;
   };
   
   std::map<int,Word*> words;

   class WordBtn
   {
   public:
      WordBtn();
      virtual ~WordBtn();

      Button * btn;
      Word * curWord;
   };
   
   std::map<int,WordBtn*> wordBtns;

   bool startedAmbientLoop;   
   SoundOGG * ambientLoopSnd;

   SoundOGG * buttonClickSnd;
   SoundOGG * errorSnd;
   
   typedef enum
   {
      GS_NONE = 0,
      GS_PATROL,
      GS_ENEMYAPPROACHING,
      GS_WORDPROMPT,
      GS_CORRECT,
      GS_EXPLODING,
      GS_INCORRECT
   } GameStateType;

   GameStateType state;

   std::vector<sprite*> enemyShips;

   long patrolCountdown;
   sprite * enemyShip;
   long preWordCountdown;
   int correctBtnIdx;
   sprite * alertLightL;
   sprite * alertLightR;
   SoundOGG * redAlertSnd;
   SoundOGG * photonSnd;
   SoundOGG * failedSnd;
   SoundOGG * explosionSnd;
   long explosionCountdown;
   std::vector<sprite*> explosions;
   sprite * photonL;
   sprite * photonR;
   
   int playerScore;
   
   int timeRemaining;
};
