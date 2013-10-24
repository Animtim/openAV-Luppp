
#include "gridlogic.hxx"

#include "jack.hxx"

#include "trackoutput.hxx"
#include "controllerupdater.hxx"

extern Jack* jack;

const char* GridLogic::StateString[8] = {
  "Empty",
  "Playing",
  "Play queued",
  "Stopped",
  "Stop queued",
  "Recording",
  "Record queued"
};

GridLogic::GridLogic()
{
  sceneLaunch = 0;
  
  sampleTrackScene = false;
  selectedTrack = 0;
  selectedScene = 0;
}

void GridLogic::selectedTrackSceneEvent(bool p)
{
  if ( p )
  {
    pressed( selectedTrack, selectedScene );
  }
  else
  {
    released( selectedTrack, selectedScene );
  }
}

void GridLogic::setSelectTrackScene(bool b)
{
  char tmp[40];
  sprintf( tmp, "Select track enable %i", int(b) );
  EventGuiPrint e( tmp );
  writeToGuiRingbuffer( &e );
  sampleTrackScene = b;
}

int GridLogic::getLaunchedScene()
{
  return sceneLaunch;
}

int GridLogic::getSelectedTrack()
{
  return selectedTrack;
}

int GridLogic::getSelectedScene()
{
  return selectedScene;
}

void GridLogic::setSelectedTrack(int t)
{
  selectedTrack = t;
}

void GridLogic::setSelectedScene(int s)
{
  selectedScene = s;
}


void GridLogic::launchScene( int scene )
{
  for(unsigned int t = 0; t < NTRACKS; t++ )
  {
    for(int s = 0; s < NSCENES; s++ )
    {
      LooperClip* lc = jack->getLooper( t )->getClip( s );
      if ( s == scene )
      {
        lc->queuePlay();
        jack->getControllerUpdater()->setSceneState( t, s, lc->getState() );
      }
      else
      {
        if ( lc->playing() )
        {
          lc->queueStop();
          jack->getControllerUpdater()->setSceneState( t, s, lc->getState() );
        }
        else if ( lc->somethingQueued() )
        {
          lc->neutralize();
          jack->getControllerUpdater()->setSceneState( t, s, lc->getState() );
        }
      }
    }
  }
  
  sceneLaunch = scene;
  
  jack->getControllerUpdater()->launchScene( scene );
}

void GridLogic::pressed( int track, int scene )
{
  if ( sampleTrackScene )
  {
    selectedTrack = track;
    selectedScene = scene;
    
    //sampleTrackScene = false;
    
    // don't act on grid press!
    return;
  }
  
  
  // get the clip, do the "press" action based on current state.
  LooperClip* lc  = jack->getLooper( track )->getClip( scene );
  TrackOutput* to = jack->getTrackOutput( track );
  GridLogic::State s = lc->getState();

#ifdef DEBUG_CLIP
  printf("GridLogic::pressed() before press state = %s\n", StateString[ int(scene) ] );
#endif 
  
  if ( to->recordArm() && !lc->recording() )
  {
    lc->queueRecord();
    to->recordArm(false);
    jack->getControllerUpdater()->recordArm( track, false );
  }
  else
  {
    if ( s == STATE_EMPTY )
      lc->queueRecord();
    
    if ( s == STATE_STOPPED )
      lc->queuePlay();
    
    if ( s == STATE_PLAYING )
      lc->queueStop();
    
    if ( s == STATE_RECORDING )
      lc->queuePlay();
    
    if ( s == STATE_PLAY_QUEUED )
      lc->queueStop();
    
    if ( s == STATE_STOP_QUEUED )
      lc->queuePlay();
    
    // don't re-trigger if already playing!
    if ( s == STATE_STOP_QUEUED && lc->playing() )
      lc->neutralize();
    
    if ( s == STATE_RECORD_QUEUED )
      lc->neutralize();
  }
  
  // check state of new clip, if getQueuePlay() == true, queueStop() all other scenes
  if ( lc->getQueuePlay() )
  {
    for(int i = 0; i < NSCENES; i++)
    {
      // exclude current scene
      if ( i != scene )
      {
        LooperClip* ilc = jack->getLooper( track )->getClip( i );
        if ( ilc->somethingQueued() )
        {
          ilc->neutralize();
          jack->getControllerUpdater()->setSceneState(track, i, ilc->getState() );
        }
        else if ( ilc->playing() )
        {
          ilc->queueStop();
          jack->getControllerUpdater()->setSceneState(track, i, ilc->getState() );
        }
      }
    }
  }
  
  
  s = lc->getState();
#ifdef DEBUG_CLIP
  printf("GridLogic::pressed() after press state = %s\n", StateString[ int(s) ] );
#endif
  jack->getControllerUpdater()->setSceneState(track, scene, s );
}


void GridLogic::released( int track, int scene )
{
  GridLogic::State s = jack->getLooper( track )->getClip( scene )->getState();
  jack->getControllerUpdater()->setSceneState(track, scene, s );
}

void GridLogic::load(int track, int scene, AudioBuffer* ab)
{
  jack->getLooper( track )->getClip( scene )->load( ab );
  GridLogic::State s = jack->getLooper( track )->getClip( scene )->getState();
  jack->getControllerUpdater()->setSceneState(track, scene, s );
}


void GridLogic::updateState()
{
  //printf("GridLogic::updateState() stub" );
  for(int t = 0; t < NTRACKS; t++)
  {
    for(int s = 0; s < NSCENES; s++)
    {
      GridLogic::State st = jack->getLooper( t )->getClip( s )->getState();
      EventGuiPrint e( GridLogic::StateString[st] );
      writeToGuiRingbuffer( &e );
      jack->getControllerUpdater()->setSceneState(t, s, st );
    }
  }
}

void GridLogic::bar()
{
#ifdef DEBUG_CLIP
  EventGuiPrint e( "GridLogic::bar()" );
  //writeToGuiRingbuffer( &e );
#endif
  
  /// iterate over all clips, if they're set to QUEUED, set to the next state
  for( int i = 0; i < NTRACKS*NSCENES; i++ )
  {
    int track = i / NSCENES;
    int scene = i - track * NSCENES;
    jack->getLooper( track )->getClip(scene)->bar();

#ifdef DEBUG_CLIP
    GridLogic::State s = jack->getLooper( track )->getClip( scene )->getState();
    if ( s != STATE_EMPTY )
    {
      //printf("%i, %i:after bar() state = %s\n", track, scene, StateString[ int(s) ] );
    }
#endif
  }
}


void GridLogic::beat()
{
  
  
}

