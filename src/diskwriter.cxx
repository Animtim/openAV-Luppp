
#include "diskwriter.hxx"

#include "config.hxx"

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

#include "worker.hxx"

using namespace std;

DiskWriter::DiskWriter()
{
};

void DiskWriter::initialize(std::string path, std::string name )
{
  sessionPath = path;
  sessionName = name;
  
  session = cJSON_CreateObject();
  sample  = cJSON_CreateObject();
  
  // add session metadata
  cJSON_AddItemToObject  ( session, "session", cJSON_CreateString( sessionName.c_str() ));
  cJSON_AddNumberToObject( session, "version_major", 1 );
  cJSON_AddNumberToObject( session, "version_minor", 0 );
  cJSON_AddNumberToObject( session, "version_patch", 0 );
  cJSON_AddNumberToObject( session, "bpm", 120 );
}

void DiskWriter::writeAudioBuffer(int track, int scene, AudioBuffer* ab )
{
  // get the filename
  stringstream filename;
  filename << "t_" << track << "_s_" << scene << ".wav";
  
  /*
  // add the track / scene / name combo to session JSON node
  cJSON* clip = cJSON_CreateObject();
  cJSON_AddItemToObject(session, "clip", clip );
  
  cJSON_AddNumberToObject(clip,"track", track);
  cJSON_AddNumberToObject(clip,"scene", scene);
  cJSON_AddStringToObject(clip,"file", filename.str().c_str());
  */
  
  // store the clip in clipData, we will write the session JSON for it in writeSession
  clipData.push_back( ClipData( track, scene, filename.str() ) );
  
  
  // add the AudioBuffer metadata to the sample JSON node
  cJSON* sampleClip = cJSON_CreateObject();
  cJSON_AddItemToObject(sample, filename.str().c_str(), sampleClip );
  cJSON_AddNumberToObject(sampleClip,"beats", ab->getBeats() );
  
  // write the AudioBuffer contents to <path>/samples/  as  <name>.wav
  // or alternatively t_<track>_s_<scene>.wav
  stringstream path;
  path << sessionPath << "/" << sessionName << "/samples/" << filename.str();
  Worker::writeSample( path.str(), ab );
  
  // de allocate the AudioBuffer
  delete ab;
}

void DiskWriter::writeSession( std::string path, std::string sessionName )
{
  // add JSON "tracks" array
  cJSON* trackArray = cJSON_CreateArray();
  cJSON_AddItemToObject(session, "tracks", trackArray );
  
  // write tracks into JSON tracks array
  for(int t = 0; t < NTRACKS; t++)
  {
    cJSON* track = cJSON_CreateObject();
    cJSON_AddItemToArray( trackArray, track );
    
    // add track metadata: volumes, sends etc
    cJSON_AddNumberToObject( track, "ID", t );
    cJSON_AddNumberToObject( track, "volume", 0.4 );
    
    // write clipData vector into clip placeholder
    cJSON* clips = cJSON_CreateArray();
    cJSON_AddItemToObject( track, "clips", clips );
    
    
    
    for(int s = 0; s < NSCENES; s++)
    {
      // add empty string to array
      cJSON* clip = cJSON_CreateString( "" );
      cJSON_AddItemToArray( clips, clip );
      
      // replace blank string if clip exists
      for(int i = 0; i < clipData.size(); i++)
      {
        if ( clipData.at(i).track == t &&
             clipData.at(i).scene == s )
        {
          cJSON* newClip = cJSON_CreateString( clipData.at(i).name.c_str() );
          cJSON_ReplaceItemInArray( clips, s, newClip );
        }
      }
      
      
    } 
    
  }
  
  
  
  // write session.luppp JSON node to <path>/<sessionName>.luppp
  stringstream sessionDir;
  sessionDir << getenv("HOME") << "/" << sessionName;
  int sessionDirError = mkdir( sessionDir.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
  if ( sessionDirError )
  {
    // handle by using different filename?
  }
  
  stringstream sampleDir;
  sampleDir << sessionDir.str() << "/samples";
  int sampleDirError  = mkdir( sampleDir.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
  
  stringstream sessionLuppp;
  sessionLuppp << sessionDir.str() << "/session.luppp";
  
  //cout << "Session dir: " << sessionDir.str() << "\n" << "Sample dir : " << sampleDir.str() << endl;
  
  ofstream sessionFile;
  sessionFile.open ( sessionLuppp.str().c_str() );
  sessionFile << cJSON_Print( session );
  sessionFile.close();
  
  
  // write the sample JSON node to <path>/samples/sample.cfg
  stringstream sampleConfig;
  sampleConfig << sampleDir.str() << "/sample.cfg";
  
  ofstream sampleFile;
  sampleFile.open ( sampleConfig.str().c_str() );
  sampleFile << cJSON_Print( sample );
  sampleFile.close();
  
  // clear the clipData, clean page for next save
  clipData.clear();
}
