
#ifndef LUPPP_METRONOME_H
#define LUPPP_METRONOME_H

#include <cmath>
#include <iostream>

#include "buffers.hxx"

#include "observer/time.hxx"

using namespace std;

// simple metronome class
class Metronome : public TimeObserver
{
  public:
    Metronome() :
      TimeObserver(),
      playBar    (false),
      active     (false),
      playPoint  (0)
    {
      // create beat and bar samples
      endPoint = (44100.f/441);
      // samples per cycle of 
      float scale = 2 * 3.1415 / endPoint;
      
      // And fill it up
      for(int i=0;i < endPoint*40;i++){
        beatSample[i]= sin(i*scale);
        barSample [i]= sin(i*scale*1.5);
      }
      
      // don't play after creation
      playPoint = endPoint + 1;
    }
    
    void setActive(bool a)
    {
      active = a;
      // don't play immidiatly
      playPoint = endPoint + 1;
    }
    
    void bar()
    {
      playPoint = 0;
      playBar = true;
    }
    
    void beat()
    {
      playPoint = 0;
      //cout << "Looper " << track << " got beat()" << flush;
    }
    
    void setFpb(int f)
    {
      fpb = f;
      
      // disable play until next beat
      playPoint = endPoint + 1;
    }
    
    void process(int nframes, Buffers* buffers)
    {
      if ( not active )
        return;
      
      float* outL = buffers->audio[Buffers::MASTER_OUT_L];
      float* outR = buffers->audio[Buffers::MASTER_OUT_R];
      
      float* sample = &beatSample[0];
      if( playBar ) { sample = &barSample[0]; playBar = false; }
      
      for(int i = 0; i < nframes; i++)
      {
        if ( playPoint < endPoint )
        {
          outL[i] += sample[playPoint];
          outR[i] += sample[playPoint];
          playPoint++;
        }
      }
    }
  
  private:
    int fpb;
    bool playBar;
    bool active;
    
    int playPoint, endPoint;
    float barSample[44100];
    float beatSample[44100];
  
};

#endif // LUPPP_METRONOME_H
