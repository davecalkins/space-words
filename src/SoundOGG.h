#pragma once

#include <cstdio>
#include <map>

struct ALOGG_OGG;

class SoundOGG
{
private:
   SoundOGG(FILE * file, ALOGG_OGG * ogg);

public:
   virtual ~SoundOGG();
   
   static SoundOGG * Load(const char * filename);

   void Play(bool loop = false);
   void Stop();

   void SetVolume(int vol);

   static void PollSounds();

private:
   int vol;
   static std::map<SoundOGG*,bool> allSounds;
   
   FILE * file;
   ALOGG_OGG * ogg;
};
