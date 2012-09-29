#define ALOGG_DLL
#include <alogg.h>

#include "env.h"
#include "SoundOGG.h"

using namespace std;

map<SoundOGG*,bool> SoundOGG::allSounds;

SoundOGG::SoundOGG(FILE * file, ALOGG_OGG * ogg)
: file(file)
, ogg(ogg)
{
   allSounds[this] = true;
}

SoundOGG::~SoundOGG()
{
   Stop();
   
   allSounds.erase(this);
   
   if (ogg != NULL)
   {
      alogg_destroy_ogg(ogg);
      ogg = NULL;
   }
   
   if (file != NULL)
   {
      fclose(file);
      file = NULL;
   }
}

SoundOGG * SoundOGG::Load(const char * filename)
{
   SoundOGG * result = NULL;
   
   FILE * file = fopen(filename,"rb");
   if (file != NULL)
   {            
      ALOGG_OGG * ogg = alogg_create_ogg_from_file(file);
      if (ogg != NULL)
      {
         result = new SoundOGG(file,ogg);
         
         if (result == NULL)
         {
            alogg_destroy_ogg(ogg);
            fclose(file);
         }
         else
            result->SetVolume(SOUNDOGG_VOLUME);
      }
      else
      {
         fclose(file);
      }
   }
   
   return result;
}

void SoundOGG::Play(bool loop /*= false*/)
{
   if (file == NULL)
      return;
      
   if (ogg == NULL)
      return;
      
   Stop();
   
   alogg_play_ex_ogg(ogg,SOUNDOGG_BUFFER_LENGTH,vol,128,1000,loop);
}

void SoundOGG::Stop()
{
   if (file == NULL)
      return;
      
   if (ogg == NULL)
      return;
      
   if (alogg_is_playing_ogg(ogg))
   {
      alogg_stop_ogg(ogg);
   }
}

void SoundOGG::SetVolume(int vol)
{
   this->vol = vol;
}

void SoundOGG::PollSounds()
{
   for (map<SoundOGG*,bool>::iterator i = allSounds.begin(); i != allSounds.end(); ++i)
   {
      SoundOGG * s = i->first;
      
      if ((s->file != NULL) && (s->ogg != NULL) && alogg_is_playing_ogg(s->ogg))
      {
         alogg_poll_ogg(s->ogg);
      }
   }
}
