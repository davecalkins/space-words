#include <allegro.h>
#include <alfont.h>

#include <cstdio>

#include "env.h"
#include "Game.h"
#include "Button.h"
#include "SoundOGG.h"
#include "sprite.h"

#define WORDBTN_EVENT_0 100

#define WORDBTN_TEXT_HEIGHT 40
#define WORDBTN_TEXT_COLOR makecol(255,255,255)

using namespace std;

char lbuf[1024];

Game::Game()
: backgroundImg(NULL)
, starfieldBImg(NULL)
, starfieldImg(NULL)
, yBPos(0)
, yPos(0)
, startedAmbientLoop(false)
, ambientLoopSnd(NULL)
, buttonClickSnd(NULL)
, errorSnd(NULL)
, state(GS_NONE)
, enemyShip(NULL)
, alertLightL(NULL)
, alertLightR(NULL)
, redAlertSnd(NULL)
, photonSnd(NULL)
, failedSnd(NULL)
, explosionSnd(NULL)
, photonL(NULL)
, photonR(NULL)
, playerScore(0)
{
}

Game::~Game()
{
}

bool Game::Init()
{
   if (!Application::Init())
      return false;

   backgroundImg = load_bitmap("data/background.bmp",NULL);
   if (backgroundImg == NULL)
      return false;

   //
   // paint the back and front starfield layers
   //
   
   starfieldBImg = create_bitmap_ex(COLOR_DEPTH,canvas->w,canvas->h);
   if (starfieldBImg == NULL)
      return false;

   rectfill(starfieldBImg,0,0,starfieldBImg->w-1,starfieldBImg->h-1,makecol(0,0,0));
   for (int i = 0; i < STARFIELDB_NUMSTARS; i++)
   {
      int x = rand() % starfieldBImg->w;
      int y = rand() % starfieldBImg->h;
      
      putpixel(starfieldBImg,x,y,STARFIELDB_COLOR1);
      rect(starfieldBImg,x-1,y-1,x+1,y+1,STARFIELDB_COLOR2);
   }

   starfieldImg = create_bitmap_ex(COLOR_DEPTH,canvas->w,canvas->h);
   if (starfieldImg == NULL)
      return false;
      
   rectfill(starfieldImg,0,0,starfieldImg->w-1,starfieldImg->h-1,makecol(255,0,255));
   for (int i = 0; i < STARFIELD_NUMSTARS; i++)
   {
      int x = rand() % starfieldImg->w;
      int y = rand() % starfieldImg->h;
      
      putpixel(starfieldImg,x,y,STARFIELD_COLOR1);
      rect(starfieldImg,x-1,y-1,x+1,y+1,STARFIELD_COLOR2);
      rect(starfieldImg,x-2,y-2,x+2,y+2,STARFIELD_COLOR3);
   }

   //
   // setup the word buttons along the right side of the screen
   //

   int x[NUM_WORD_BTNS] = { 790, 790, 790, 790, 790, 790 };
   int y[NUM_WORD_BTNS] = {  28, 133, 239, 346, 451, 557 };

   for (int i = 0; i < NUM_WORD_BTNS; i++)
   {
      WordBtn * wordBtn = new WordBtn;
      wordBtn->btn = new Button("data/wordbtn.bmp","data/wordbtn_mouseover.bmp","",x[i],y[i],0,WORDBTN_EVENT_0+i,true,false);
      if (wordBtn->btn == NULL)
         return false;
      if (!wordBtn->btn->Init())
         return false;
      wordBtns[i] = wordBtn;
   }
   
   ambientLoopSnd = SoundOGG::Load("data/ambientloop.ogg");
   if (ambientLoopSnd == NULL)
      return false;
      
   buttonClickSnd = SoundOGG::Load("data/buttonclick.ogg");
   if (buttonClickSnd == NULL)
      return false;      
   
   errorSnd = SoundOGG::Load("data/error.ogg");
   if (errorSnd == NULL)
      return false;      

   //
   // load the words file and for each word load its associated sound file
   //   

   FILE * wf = fopen("data/words.txt","r");
   if (wf == NULL)
      return false;
   int curWordIdx = 0;
   while (!feof(wf))
   {
      char buf[4096];
      if (fgets(buf,4096,wf) == NULL)
      {
         log("reached the end of the words file\n");
         break;
      }

      int blen = strlen(buf);
      if (buf[blen-1] == '\n')
         buf[blen-1] = '\0';

      if (strlen(buf) == 0)
         continue;

      if (buf[0] == '#')
         continue;
      
      string text = buf;
      
      char sfbuf[MAX_PATH+1];
      sprintf(sfbuf,"data/word_%s.ogg",text.c_str());

      SoundOGG * sound = SoundOGG::Load(sfbuf);
      if (sound == NULL)
      {
         sprintf(lbuf,"ERROR: failed to load sound file for word '%s'\n",text.c_str());
         log(lbuf);
         continue;
      }

      sound->SetVolume(WORD_VOLUME);
         
      Word * w = new Word;
      w->text = text;
      w->sound = sound;

      words[curWordIdx++] = w;

      sprintf(lbuf,"loaded word '%s' sound '%s'\n",text.c_str(),sfbuf);
      log(lbuf);      
   }      
   fclose(wf);
   
   //
   // enemy ships
   //

   for (int shipNum = 1; ; shipNum++)
   {
      char fbuf[MAX_PATH];
      sprintf(fbuf,"data/enemy%d.bmp",shipNum);
      
      if (exists(fbuf))
      {
         BITMAP * tmpBmp = load_bitmap(fbuf,NULL);
         if (!tmpBmp)
         {
            sprintf(lbuf,"ERROR: failed to load %s\n",fbuf);            
            return false;
         }
         
         int imgWidth = tmpBmp->w;
         int imgHeight = tmpBmp->h;
         
         destroy_bitmap(tmpBmp);
         
         sprite * s = new sprite;
         if (s == NULL)
            return false;
            
         if (!s->load(fbuf))
         {
            sprintf(lbuf,"ERROR: failed to load %s\n",fbuf);
            s->destroy();
            return false;
         }
         else
         {
            s->width = imgWidth;
            s->height = imgHeight;

            enemyShips.push_back(s);
         }
      }
      else
         break;
   }
   
   if (enemyShips.size() == 0)
   {
      log("ERROR: no enemy ships loaded\n");
      return false;
   }

   //
   // alert lights
   //
   
   alertLightL = new sprite;
   if (alertLightL == NULL)
      return false;
   if (!alertLightL->load("data/alertlight.bmp"))
   {
      alertLightL->destroy();
      return false;
   }
   alertLightL->width = ALERTLIGHT_WIDTH;
   alertLightL->height = ALERTLIGHT_HEIGHT;
   alertLightL->x = ALERTLIGHTL_X;
   alertLightL->y = ALERTLIGHTL_Y;
   alertLightL->totalframes = ALERTLIGHT_TOTALFRAMES;
   alertLightL->framedelay = ALERTLIGHT_FRAMEDELAY;

   alertLightR = new sprite;
   if (alertLightR == NULL)
      return false;
   if (!alertLightR->load("data/alertlight.bmp"))
   {
      alertLightR->destroy();
      return false;
   }
   alertLightR->width = ALERTLIGHT_WIDTH;
   alertLightR->height = ALERTLIGHT_HEIGHT;
   alertLightR->x = ALERTLIGHTR_X;
   alertLightR->y = ALERTLIGHTR_Y;
   alertLightR->totalframes = ALERTLIGHT_TOTALFRAMES;
   alertLightR->framedelay = ALERTLIGHT_FRAMEDELAY;

   // red alert sound
   redAlertSnd = SoundOGG::Load("data/redalert.ogg");
   if (redAlertSnd == NULL)
      return false;
      
   // photon torpedo sound
   photonSnd = SoundOGG::Load("data/photon.ogg");
   if (photonSnd == NULL)
      return false;

   // failed sound
   failedSnd = SoundOGG::Load("data/failed.ogg");
   if (failedSnd == NULL)
      return false;

   // explosion sound
   explosionSnd = SoundOGG::Load("data/explosion.ogg");
   if (explosionSnd == NULL)
      return false;

   // explosion sprites
   for (int i = 0; i < NUM_EXPLOSION_SPRITES; i++)
   {
      sprite * ex = new sprite;
      if (ex == NULL)
         return false;
      if (!ex->load("data/explosion.bmp"))
         return false;
      ex->width = EXPLOSION_FRAME_WIDTH;
      ex->height = EXPLOSION_FRAME_HEIGHT;
      ex->totalframes = EXPLOSION_NUM_FRAMES;
      ex->animcolumns = EXPLOSION_NUM_FRAMES;
      ex->framedelay = EXPLOSION_FRAME_DELAY;
      
      explosions.push_back(ex);
   }

   // photon sprites
   photonL = new sprite;
   if (photonL == NULL)
      return false;
   if (!photonL->load("data/photon.bmp"))
      return false;
   photonL->width = PHOTON_FRAME_WIDTH;
   photonL->height = PHOTON_FRAME_HEIGHT;
   photonL->totalframes = PHOTON_NUM_FRAMES;
   photonL->animcolumns = PHOTON_NUM_FRAMES;
   photonL->framedelay = PHOTON_FRAME_DELAY;
   photonR = new sprite;
   if (photonR == NULL)
      return false;
   if (!photonR->load("data/photon.bmp"))
      return false;
   photonR->width = PHOTON_FRAME_WIDTH;
   photonR->height = PHOTON_FRAME_HEIGHT;
   photonR->totalframes = PHOTON_NUM_FRAMES;
   photonR->animcolumns = PHOTON_NUM_FRAMES;
   photonR->framedelay = PHOTON_FRAME_DELAY;

   // initial state
   state = GS_PATROL;
   patrolCountdown = PATROL_DURATION_SECS * DESIRED_FPS;

   return true;
}

void Game::Destroy()
{
   if (backgroundImg != NULL)
   {
      destroy_bitmap(backgroundImg);
      backgroundImg = NULL;
   }

   for (map<int,WordBtn*>::iterator i = wordBtns.begin(); i != wordBtns.end(); ++i)
   {
      delete i->second;
   }
   wordBtns.clear();
   
   if (ambientLoopSnd != NULL)
   {
      delete ambientLoopSnd;
      ambientLoopSnd = NULL;
   }
   
   if (buttonClickSnd != NULL)
   {
      delete buttonClickSnd;
      buttonClickSnd = NULL;
   }
   
   if (errorSnd != NULL)
   {
      delete errorSnd;
      errorSnd = NULL;
   }

   for (map<int,Word*>::iterator i = words.begin(); i != words.end(); ++i)
   {
      delete i->second;
   }
   words.clear();
   
   for (vector<sprite*>::iterator i = enemyShips.begin(); i != enemyShips.end(); ++i)
   {
      (*i)->destroy();
   }
   enemyShips.clear();
   
   if (alertLightL != NULL)
   {
      alertLightL->destroy();
      alertLightL = NULL;
   }

   if (alertLightR != NULL)
   {
      alertLightR->destroy();
      alertLightR = NULL;
   }

   if (redAlertSnd != NULL)
   {
      delete redAlertSnd;
      redAlertSnd = NULL;
   }

   if (photonSnd != NULL)
   {
      delete photonSnd;
      photonSnd = NULL;
   }
   
   if (failedSnd != NULL)
   {
      delete failedSnd;
      failedSnd = NULL;
   }

   if (explosionSnd != NULL)
   {
      delete explosionSnd;
      explosionSnd = NULL;
   }

   for (vector<sprite*>::iterator i = explosions.begin(); i != explosions.end(); ++i)
   {
      (*i)->destroy();
   }
   explosions.clear();

   if (photonL != NULL)
   {
      photonL->destroy();
      photonL = NULL;
   }

   if (photonR != NULL)
   {
      photonR->destroy();
      photonR = NULL;
   }

   Application::Destroy();
}

void Game::RunGame()
{  
   // ensure ambient loop is running
   if (!startedAmbientLoop)
   {
      ambientLoopSnd->Play(true);
      startedAmbientLoop = true;
   } 
   
   //
   // scrolling starfield
   //

   // distant starfield (B)ack starfield   
   blit(starfieldBImg,canvas,0,0,0,yBPos,starfieldBImg->w,starfieldBImg->h - yBPos);
   if (yBPos > 0)
      blit(starfieldBImg,canvas,0,starfieldBImg->h-yBPos,0,0,starfieldBImg->w,yBPos);

   // close starfield, foreground stars
   masked_blit(starfieldImg,canvas,0,0,0,yPos,starfieldImg->w,starfieldImg->h - yPos);
   if (yPos > 0)
      masked_blit(starfieldImg,canvas,0,starfieldImg->h-yPos,0,0,starfieldImg->w,yPos);

   // scroll both starfields   
   yBPos += SCROLLB_SPEED;
   if (yBPos >= canvas->h)
      yBPos = 0;
   yPos += SCROLL_SPEED;
   if (yPos >= canvas->h)
      yPos = 0;

   // enemy ship
   if (enemyShip != NULL)
   {
      enemyShip->draw(canvas);
      enemyShip->move();
   }

   // main background image (starfield area is transparent to let the stars show through)
   masked_blit(backgroundImg,canvas,0,0,0,0,backgroundImg->w,backgroundImg->h);     

   // alert lights
   alertLightL->drawframe(canvas);
   alertLightR->drawframe(canvas);
   
   // word buttons
   alfont_set_font_size(font,WORDBTN_TEXT_HEIGHT);
   for (map<int,WordBtn*>::iterator i = wordBtns.begin(); i != wordBtns.end(); ++i)
   {
      WordBtn * wb = i->second;
      
      wb->btn->Run(canvas);
      
      if (wb->curWord != NULL)
      {
         alfont_textout_centre_ex(canvas,font,wb->curWord->text.c_str(),wb->btn->GetX() + wb->btn->GetWidth()/2,wb->btn->GetY()+8,WORDBTN_TEXT_COLOR,-1);
      }
   }
   
   // player score
   char sbuf[MAX_PATH];
   sprintf(sbuf,"Score: %d",playerScore);
   alfont_set_font_size(font,SCORE_HEIGHT);
   alfont_textprintf_ex(canvas,font,SCORE_X,SCORE_Y,SCORE_COLOR,-1,sbuf);
   
   //
   // state behavior
   //
   
   switch (state)
   {
   case GS_PATROL:
      {
         alertLightL->curframe = 0;
         alertLightR->curframe = 0;
                  
         if (patrolCountdown > 0)
            patrolCountdown--;
            
         if (patrolCountdown == 0)
         {
            state = GS_ENEMYAPPROACHING;
            
            enemyShip = enemyShips[rand() % enemyShips.size()];
            enemyShip->x = STARFIELD_LEFTEDGE + (int)((double)(STARFIELD_WIDTH - enemyShip->width) / 2.0);
            enemyShip->y = STARFIELD_TOPEDGE - enemyShip->height;
            enemyShip->velx = 0;
            enemyShip->vely = ENEMYSHIP_SPEED;
            
            redAlertSnd->Play();
         }
      }
      break;

   case GS_ENEMYAPPROACHING:
      {
         alertLightL->animate();
         alertLightR->animate();
         
         if (enemyShip->vely == 0)
         {
            if (preWordCountdown > 0)
               preWordCountdown--;
               
            if (preWordCountdown == 0)
            {
               state = GS_WORDPROMPT;
   
               Word * curWord;
               map<Word*,bool> usedWords;
               map<WordBtn*,bool> usedWordBtns;
               for (int i = 0; i < NUM_DISPLAYED_WORDS_AT_ONCE; i++)
               {
                  Word * w;
                  do
                  {
                     w = words[rand() % words.size()];
                     if (i == 0)
                        curWord = w;
                  } while (usedWords.find(w) != usedWords.end());   
                  usedWords[w] = true;
   
                  WordBtn * wb;
                  do
                  {
                     int btnIdx = rand() % wordBtns.size();
                     if (i == 0)
                        correctBtnIdx = btnIdx;
                     wb = wordBtns[btnIdx];
                  } while (usedWordBtns.find(wb) != usedWordBtns.end());
                  usedWordBtns[wb] = true;
   
                  wb->curWord = w;
                  wb->btn->setVisible(true);
               }
               
               curWord->sound->Play();            
               
               timeRemaining = TOTAL_TIME_ALLOWED;
            }
         }
         else if ((enemyShip->y + (int)((double)enemyShip->height / 2.0)) >= (STARFIELD_TOPEDGE + (int)((double)STARFIELD_HEIGHT/2.0)))
         {
            enemyShip->vely = 0;
            preWordCountdown = PREWORD_DELAY_SECS * DESIRED_FPS;
         }
      }
      break;  
      
   case GS_WORDPROMPT:
      {
         alertLightL->animate();  
         alertLightR->animate();  
         
         /* TIMEOUT FEATURE -- DISABLED
         rect(canvas,TIMELEFT_X,TIMELEFT_Y,TIMELEFT_X+TIMELEFT_WIDTH-1,TIMELEFT_Y+TIMELEFT_HEIGHT-1,TIMELEFT_BORDER_COLOR);
         
         int trWidth = (int)(((double)timeRemaining / (double)TOTAL_TIME_ALLOWED) * (double)(TIMELEFT_WIDTH-2));
         if (trWidth > 0)
            rectfill(canvas,TIMELEFT_X+1,TIMELEFT_Y+1,TIMELEFT_X+1+trWidth-1,TIMELEFT_Y+TIMELEFT_HEIGHT-2,TIMELEFT_FILL_COLOR);
         
         timeRemaining--;
         if (timeRemaining < 0)
         {
            timeRemaining = 0;               

            int selBtn = 0;
            if (selBtn == correctBtnIdx)
               selBtn++;
            
            BroadcastEvent(WORDBTN_EVENT_0+selBtn);            
         }*/
      }
      break;  
      
   case GS_CORRECT:
      {
         alertLightL->animate(); 
         alertLightR->animate(); 

         photonL->drawframe(canvas);
         if (photonL->curframe < (PHOTON_NUM_FRAMES-1))
            photonL->animate();
         if (photonL->x < (STARFIELD_LEFTEDGE + (STARFIELD_WIDTH/2) - (photonL->width/2)))
            photonL->move();
         
         photonR->drawframe(canvas);
         if (photonR->curframe < (PHOTON_NUM_FRAMES-1))
            photonR->animate();
         if (photonR->x > (STARFIELD_LEFTEDGE + (STARFIELD_WIDTH/2) - (photonL->width/2)))
            photonR->move();         

         explosionCountdown--;
         
         if (explosionCountdown == 0)
         {
            explosionSnd->Play();
            
            state = GS_EXPLODING;
            
            int minX = (int)enemyShip->x;
            int minY = (int)enemyShip->y;
            int maxX = (int)enemyShip->x + enemyShip->width - EXPLOSION_FRAME_WIDTH;
            int maxY = (int)enemyShip->y + enemyShip->height - EXPLOSION_FRAME_HEIGHT;
            
            for (vector<sprite*>::iterator i = explosions.begin(); i != explosions.end(); ++i)
            {
               (*i)->x = minX + (rand() % (maxX-minX+1));
               (*i)->y = minY + (rand() % (maxY-minY+1));               
               (*i)->curframe = 0;
            }
         }
      }
      break;  

   case GS_EXPLODING:
      {
         bool doneExploding = false;
         
         for (vector<sprite*>::iterator i = explosions.begin(); i != explosions.end(); ++i)
         {
            (*i)->drawframe(canvas);
            (*i)->animate();
            if ((*i)->curframe >= (EXPLOSION_NUM_FRAMES-1))
               doneExploding = true;
         }
         
         if (doneExploding)
         {
            state = GS_PATROL;
            patrolCountdown = PATROL_DURATION_SECS * DESIRED_FPS;
            enemyShip = NULL;

            for (int i = 0; i < NUM_WORD_BTNS; i++)
            {
               wordBtns[i]->btn->setVisible(false);
               wordBtns[i]->curWord = NULL;
            }
         }

         alertLightL->curframe = 0;
         alertLightR->curframe = 0;
      }
      break;

   case GS_INCORRECT:
      {
         alertLightL->animate();
         alertLightR->animate();
         enemyShip->move();
         
         if (enemyShip->y <= (STARFIELD_TOPEDGE - enemyShip->height))
         {
            alertLightL->curframe = 0;
            alertLightR->curframe = 0;
            
            for (int i = 0; i < NUM_WORD_BTNS; i++)
            {
               wordBtns[i]->btn->setVisible(false);
               wordBtns[i]->curWord = NULL;
            }
            
            state = GS_PATROL;
            patrolCountdown = PATROL_DURATION_SECS * DESIRED_FPS;
            enemyShip = NULL;
         }
      }
      break;        
   }
}

void Game::OnKeyPress(int keyCode)
{
}

void Game::OnKeyPressed(int keyCode)
{
}

void Game::OnKeyReleased(int keyCode)
{
   if (keyCode == KEY_ESC)
      KeepRunning = false;
}

void Game::OnMouseMove(int x, int y)
{
   // word buttons
   for (map<int,WordBtn*>::iterator i = wordBtns.begin(); i != wordBtns.end(); ++i)
   {
      i->second->btn->OnMouseMove(x,y);
   }
}

void Game::OnMouseClick(int button, int x, int y)
{
}

void Game::OnMousePressed(int button, int x, int y)
{
}

void Game::OnMouseReleased(int button, int x, int y)
{
   // word buttons
   for (map<int,WordBtn*>::iterator i = wordBtns.begin(); i != wordBtns.end(); ++i)
   {
      i->second->btn->OnMouseReleased(button,x,y);
   }
}

void Game::OnMouseWheelUp(int x, int y)
{
}

void Game::OnMouseWheelDown(int x, int y)
{
}

void Game::OnEvent(int eventID)
{
   if ((state == GS_WORDPROMPT) && (eventID >= WORDBTN_EVENT_0) && (eventID < (WORDBTN_EVENT_0 + NUM_WORD_BTNS)))
   {
      buttonClickSnd->Play();
      
      int btnIdx = eventID - WORDBTN_EVENT_0;
      
      //
      // CORRECT ANSWER!!
      //
      
      if (btnIdx == correctBtnIdx)
      {
         photonSnd->Play();
         state = GS_CORRECT;
         
         for (int i = 0; i < NUM_WORD_BTNS; i++)
         {
            if (i != btnIdx)
            {
               wordBtns[i]->btn->setVisible(false);
               wordBtns[i]->curWord = NULL;
            }
         }

         photonL->x = STARFIELD_LEFTEDGE;
         photonL->y = STARFIELD_TOPEDGE + (STARFIELD_HEIGHT/2) - (photonL->height/2);
         photonL->curframe = 0;
         photonL->velx = PHOTON_SPEED;

         photonR->x = STARFIELD_LEFTEDGE + STARFIELD_WIDTH - photonR->width;
         photonR->y = STARFIELD_TOPEDGE + (STARFIELD_HEIGHT/2) - (photonL->height/2);
         photonR->curframe = 0;
         photonR->velx = -PHOTON_SPEED;
         
         explosionCountdown = EXPLOSION_DELAY_SECS * DESIRED_FPS;                 
         
         playerScore += PTS_PER_LETTER * (wordBtns[btnIdx]->curWord->text.size());
      }      

      //
      // INCORRECT ANSWER
      //
      
      else
      {
         failedSnd->Play();
         state = GS_INCORRECT;
         
         enemyShip->vely = -(ENEMYSHIP_SPEED * 2);

         playerScore -= PTS_PER_LETTER * (wordBtns[btnIdx]->curWord->text.size());
         if (playerScore < 0)
            playerScore = 0;
      }
   }
}

Game::WordBtn::WordBtn()
: btn(NULL)
, curWord(NULL)
{
}

Game::WordBtn::~WordBtn()
{
   if (btn != NULL)
   {
      btn->Destroy();
      delete btn;
      btn = NULL;
   }
}

Game::Word::Word()
: sound(NULL)
{
}

Game::Word::~Word()
{
   if (sound != NULL)
   {
      delete sound;
      sound = NULL;
   }
}
