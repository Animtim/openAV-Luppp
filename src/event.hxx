
#ifndef LUPPP_EVENT_H
#define LUPPP_EVENT_H

#include <iostream>
#include <stdint.h>

/*
    event.hxx
  
  This file provides declarations for each type of event that the engine uses.
  
*/

#include "looper.hxx"
#include "gridlogic.hxx"

using namespace std;

namespace Event
{
  enum SEND_TYPE
  {
    SEND_REV = 0,
    SEND_SIDE,
    SEND_POST,
  };
  
  enum {
    MASTER_VOL,
    RECORD,
    
    SAVE,       // save action
    SAVE_FINISH,// save action finished, flush metadata to disk
    SAVE_BUFFER,// save an individual AudioBuffer* to disk
    
    REQUEST_SAVE_BUFFER, // gets an audioBuffer of a certain size
    
    GRID_EVENT, // press / release events
    GRID_STATE, // state of one block
    GRID_LAUNCH_SCENE,
    
    TRACK_SEND,
    TRACK_SIGNAL_LEVEL,
    TRACK_VOLUME,
    
    FX_REVERB,
    
    LOOPER_LOAD,
    LOOPER_STATE,
    LOOPER_PROGRESS,
    LOOPER_LOOP_LENGTH,
    LOOPER_LOOP_USE_AS_TEMPO,
    
    METRONOME_ACTIVE,
    
    TIME_BPM,
    TIME_BAR_BEAT,
    TIME_TEMPO_TAP,
    
    GUI_PRINT,
    
    LOOPER_REQUEST_BUFFER,
    DEALLOCATE_BUFFER,
  };
};

using namespace Event;

class AudioBuffer;

class EventBase
{
  public:
    virtual ~EventBase() {}
    
    virtual int type() = 0;
    virtual uint32_t size() = 0;
};

class EventMasterVol : public EventBase
{
  public:
    int type() { return int(MASTER_VOL); }
    uint32_t size() { return sizeof(EventMasterVol); }
    float vol;
    
    EventMasterVol(float v)
    {
      vol = v;
    }
};

class EventTrackVol : public EventBase
{
  public:
    int type() { return int(TRACK_VOLUME); }
    uint32_t size() { return sizeof(EventTrackVol); }
    
    int track;
    float vol;
    
    EventTrackVol(){};
    EventTrackVol(int t, float v)
    {
      track = t;
      vol = v;
    }
};

class EventSave : public EventBase
{
  public:
    int type() { return int(SAVE); }
    uint32_t size() { return sizeof(EventSave); }
    
    int waste;
    
    EventSave() : waste(0) {}
};

class EventSaveFinish : public EventBase
{
  public:
    int type() { return int(SAVE_FINISH); }
    uint32_t size() { return sizeof(EventSaveFinish); }
    
    EventSaveFinish(){};
};

class EventGridEvent : public EventBase
{
  public:
    int type() { return int(GRID_EVENT); }
    uint32_t size() { return sizeof(EventGridEvent); }
    
    int track;
    int scene;
    bool pressed;
    
    EventGridEvent(){};
    EventGridEvent(int t, int s, bool p): track(t), scene(s), pressed(p) {}
};

class EventGridState : public EventBase
{
  public:
    int type() { return int(GRID_STATE); }
    uint32_t size() { return sizeof(EventGridState); }
    
    int track;
    int scene;
    GridLogic::State state;
    
    EventGridState(){};
    EventGridState(int t, int s, GridLogic::State st): track(t), scene(s), state(st) {}
};

class EventGridLaunchScene : public EventBase
{
  public:
    int type() { return int(GRID_LAUNCH_SCENE); }
    uint32_t size() { return sizeof(EventGridLaunchScene); }
    
    int scene;
    
    EventGridLaunchScene(){};
    EventGridLaunchScene(int s): scene(s){}
};

class EventFxReverb : public EventBase
{
  public:
    int type() { return int(FX_REVERB); }
    uint32_t size() { return sizeof(EventFxReverb); }
    
    int track;
    bool enable;
    float damping;
    float rtSize;
    
    EventFxReverb(){};
    EventFxReverb(int t, bool e, float d, float s): track(t), enable(e), damping(d), rtSize(s) {}
};

class EventTrackSend : public EventBase
{
  public:
    int type() { return int(TRACK_SEND); }
    uint32_t size() { return sizeof(EventTrackSend); }
    
    int track;
    SEND_TYPE send;
    float value;
    
    EventTrackSend(){};
    EventTrackSend(int t, SEND_TYPE s, float v): track(t), send(s), value(v){}
};

class EventLooperState : public EventBase
{
  public:
    int type() { return int(LOOPER_STATE); }
    uint32_t size() { return sizeof(EventLooperState); }
    
    int track;
    int scene;
    //Looper::State state;
    
    EventLooperState(){}
    //EventLooperState(int t, int sc, Looper::State s) : track(t), scene(sc), state(s){}
};

class EventLooperProgress : public EventBase
{
  public:
    int type() { return int(LOOPER_PROGRESS); }
    uint32_t size() { return sizeof(EventLooperProgress); }
    
    int track;
    float progress;
    EventLooperProgress(){}
    EventLooperProgress(int t, float p) : track(t), progress(p) {}
};

class EventLooperLoopLength : public EventBase
{
  public:
    int type() { return int(LOOPER_LOOP_LENGTH); }
    uint32_t size() { return sizeof(EventLooperLoopLength); }
    
    int track;
    int scene;
    int beats;
    EventLooperLoopLength(){}
    EventLooperLoopLength(int t, int s, int b) : track(t), scene(s), beats(b) {}
};

class EventLooperUseAsTempo : public EventBase
{
  public:
    int type() { return int(LOOPER_LOOP_USE_AS_TEMPO); }
    uint32_t size() { return sizeof(EventLooperUseAsTempo); }
    
    int track;
    int scene;
    int beats;
    EventLooperUseAsTempo(){}
    EventLooperUseAsTempo(int t, int s) : track(t), scene(s){}
};

class EventLooperLoad : public EventBase
{
  public:
    int type() { return int(LOOPER_LOAD); }
    uint32_t size() { return sizeof(EventLooperLoad); }
    
    int track;
    int clip;
    void* audioBuffer;
    
    EventLooperLoad(){}
    EventLooperLoad(int t, int c, void* ab) : track(t), clip(c), audioBuffer(ab)
    {
      cout << "ab ptr = " << audioBuffer << endl;
    }
};

class EventMetronomeActive : public EventBase
{
  public:
    int type() { return int(METRONOME_ACTIVE); }
    uint32_t size() { return sizeof(EventMetronomeActive); }
    
    bool active;
    
    EventMetronomeActive() : active(false) {}
    EventMetronomeActive(bool a) : active(a) {}
};

class EventTimeBPM : public EventBase
{
  public:
    int type() { return int(TIME_BPM); }
    uint32_t size() { return sizeof(EventTimeBPM); }
    
    float bpm;
    
    EventTimeBPM(){}
    EventTimeBPM(float b) : bpm(b) {}
};

class EventTrackSignalLevel : public EventBase
{
  public:
    int type() { return int(TRACK_SIGNAL_LEVEL); }
    uint32_t size() { return sizeof(EventTrackSignalLevel); }
    
    int track;
    float left;
    float right;
    
    EventTrackSignalLevel(){}
    EventTrackSignalLevel(int t, float l,float r) : track(t), left(l), right(r) {}
};

class EventTimeTempoTap : public EventBase
{
  public:
    int type() { return int(TIME_TEMPO_TAP); }
    uint32_t size() { return sizeof(EventTimeTempoTap); }
    
    bool pressed; // only used to turn on / off the button in GUI
    EventTimeTempoTap():pressed(false){}
    EventTimeTempoTap(bool p): pressed(p) {}
};

class EventTimeBarBeat : public EventBase
{
  public:
    int type() { return int(TIME_BAR_BEAT); }
    uint32_t size() { return sizeof(EventTimeBarBeat); }
    
    int bar;
    int beat;
    EventTimeBarBeat(): bar(0), beat(0){}
    EventTimeBarBeat(int ba, int be): bar(ba), beat(be) {}
};

class EventLooperClipRequestBuffer : public EventBase
{
  public:
    int type() { return int(LOOPER_REQUEST_BUFFER); }
    uint32_t size() { return sizeof(EventLooperClipRequestBuffer); }
    
    int track;
    int scene;
    
    // number of floats to contain
    unsigned long numElements;
    
    // pointer to the new AudioBuffer being passed back
    AudioBuffer* ab;
    
    EventLooperClipRequestBuffer(): track(0), scene(0), numElements(0), ab(0) {}
    EventLooperClipRequestBuffer(int t, int s, int siz): track(t), scene(s), numElements(siz), ab(0) {}
    EventLooperClipRequestBuffer(int t, int s, AudioBuffer* a): track(t), scene(s), numElements(0), ab(a) {}
};


class EventSaveBuffer : public EventBase
{
  public:
    int type() { return int(SAVE_BUFFER); }
    uint32_t size() { return sizeof(EventSaveBuffer); }
    
    int track;
    int scene;
    // pointer to the AudioBuffer to be saved
    AudioBuffer* ab;
    
    EventSaveBuffer(): track(0), scene(0), ab(0) {}
    EventSaveBuffer(int t, int s, AudioBuffer* a): track(t), scene(s), ab(a) {}
};

class EventRequestSaveBuffer : public EventBase
{
  public:
    int type() { return int(REQUEST_SAVE_BUFFER); }
    uint32_t size() { return sizeof(EventRequestSaveBuffer); }
    
    int track;
    int scene;
    
    size_t bufferSize;
    
    // pointer to the AudioBuffer to be saved
    AudioBuffer* ab;
    
    EventRequestSaveBuffer(): track(0), scene(0), ab(0) {}
    EventRequestSaveBuffer(int t, int s, size_t si): track(t), scene(s), bufferSize(si), ab(0) {}
    EventRequestSaveBuffer(int t, int s, AudioBuffer* a): track(t), scene(s), ab(a) {}
};

class EventDeallocateBuffer : public EventBase
{
  public:
    int type() { return int(DEALLOCATE_BUFFER); }
    uint32_t size() { return sizeof(EventDeallocateBuffer); }
    
    AudioBuffer* ab;
    
    EventDeallocateBuffer(): ab(0) {}
    EventDeallocateBuffer(AudioBuffer* a): ab(a) {}
};


// prints the string S in the GUI console
class EventGuiPrint : public EventBase
{
  public:
    int type() { return int(GUI_PRINT); }
    uint32_t size() { return sizeof(EventGuiPrint); }
    
    char stringArray[50];
    
    EventGuiPrint(){}
    EventGuiPrint(const char* s)
    {
      if ( strlen( s ) > 50 )
      {
        // this will be called from an RT context, and should be removed from
        // production code. It is here for the programmer to notice when they
        // are using code which causes too long a message.
        cout << "EventGuiPrint() error! Size of string too long!" << endl;
      }
      else
      {
        // move the sting into this event
        strcpy( &stringArray[0], s );
      }
    }
    char* getMessage()
    {
      return &stringArray[0];
    }
};

#endif // LUPPP_EVENT_H

