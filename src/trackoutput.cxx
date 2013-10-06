
#include "trackoutput.hxx"

#include "jack.hxx"
extern Jack* jack;

TrackOutput::TrackOutput(int t, AudioProcessor* ap) :
  AudioProcessor(),
  track(t),
  _recordArm(false),
  previousInChain(ap)
{
  // UI update
  uiUpdateConstant = jack->getSamplerate() / 30;
  uiUpdateCounter  = jack->getSamplerate() / 30;
  
  dbMeter = new DBMeter( jack->getSamplerate() );
  
  _toMaster        = 0.8;
  
  _toReverb        = 0.0;
  _toSidechain     = 0.0;
  _toPostSidechain = 0.0;
  
  _toPostfaderActive        = 0;
  _toKeyActive     = 0;
  _toXSideActive = true;
}


void TrackOutput::setMaster(float value)
{
  _toMaster = value;
}

float TrackOutput::getMaster()
{
  return _toMaster;
}


bool TrackOutput::recordArm()
{
  return _recordArm;
}

void TrackOutput::recordArm(bool r)
{
  _recordArm = r;
}

void TrackOutput::setSendActive( int send, bool a )
{
  switch( send )
  {
    case SEND_POSTFADER:
        _toPostfaderActive = a;
        break;
    case SEND_KEY:
        _toKeyActive = a;
        break;
    //case SEND_XSIDE:
    //    _toXSideActive = a;
    default:
        break;
  }
}

void TrackOutput::setSend( int send, float value )
{
  switch( send )
  {
    case SEND_POSTFADER:
        _toReverb = value;
        break;
    case SEND_KEY:
        // setSendActive() handles on/off for this send
        //_toSidechain = value;
        break;
    case SEND_XSIDE:
        _toPostSidechain = value;
        break;
  }
}

void TrackOutput::process(unsigned int nframes, Buffers* buffers)
{
  // get & zero track buffer
  float* trackBuffer = buffers->audio[Buffers::TRACK_0 + track];
  memset( trackBuffer, 0, sizeof(float)*nframes );
  
  // call process() up the chain
  previousInChain->process( nframes, buffers );
  
  // run the meter
  dbMeter->process( nframes, trackBuffer, trackBuffer );
  
  if (uiUpdateCounter > uiUpdateConstant )
  {
    float l = dbMeter->getLeftDB() * _toMaster;
    float r = dbMeter->getRightDB() * _toMaster;
    EventTrackSignalLevel e( track, l, r );
    writeToGuiRingbuffer( &e );
    uiUpdateCounter = 0;
  }
  
  uiUpdateCounter += nframes;
  
  // copy audio data into reverb / sidechain / master buffers
  float* reverb        = buffers->audio[Buffers::SEND];
  float* sidechain     = buffers->audio[Buffers::SIDECHAIN_KEY];
  float* postSidechain = buffers->audio[Buffers::SIDECHAIN_SIGNAL];
  
  float* masterL       = buffers->audio[Buffers::MASTER_OUT_L];
  float* masterR       = buffers->audio[Buffers::MASTER_OUT_R];
  
  for(unsigned int i = 0; i < nframes; i++)
  {
    // * master for "post-fader" sends
    float tmp = trackBuffer[i];
    
    // post-sidechain *moves* signal between "before/after" ducking, not add!
    masterL[i]       += tmp * _toMaster * (1-_toPostSidechain);
    masterR[i]       += tmp * _toMaster * (1-_toPostSidechain);
    
    if ( _toPostfaderActive )
      reverb[i]        += tmp * _toReverb * _toMaster;
    
    if ( _toXSideActive )
      postSidechain[i] += tmp * _toPostSidechain * _toMaster;
    
    // turning down an element in the mix should *NOT* influence sidechaining
    if ( _toKeyActive )
      sidechain[i]     += tmp;
    
  }
}

TrackOutput::~TrackOutput()
{
  delete dbMeter;
}
