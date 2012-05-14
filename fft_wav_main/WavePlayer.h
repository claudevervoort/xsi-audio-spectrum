#pragma once

#ifndef FFTW_WAVE_PLAYER
#define FFTW_WAVE_PLAYER

#include "../fft_wav_analyzer/WaveLoader.h"

namespace FFTWave
{
  class WavePlayer
  {
  public:
	  WavePlayer(WaveLoader* waveLoader );
	  virtual ~WavePlayer(void);

    static const int PLAY_QUEUE_SIZE = 10;
    static const int FREQUENCY = 30; // how often a callback occurs, per second

	  BOOL open();
    BOOL play();
	  BOOL stop();
	  BOOL pause();
	  BOOL isPlaying()			{return (!_bStopped && !_bPaused);}
	  BOOL isStopped()			{return _bStopped;}
	  BOOL isPaused()				{return _bPaused;}
    BOOL prepareAndQueueNextHeader( int headerNum, BOOL init=FALSE );
    int  getCurrentHeader( ) {return _currentHeader;}
    int getNumBlocksPerHeader( ) {return _numSampleSlicesPerHeader; }

  private:
	  // Private methods
	  BOOL isValid()				{return (_waveLoader != NULL);}
	  BOOL close();
	  void static CALLBACK waveOut_Proc_Streaming(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	  // Private members
    WaveLoader* _waveLoader;
	  HWAVEOUT _hWaveout;
    WAVEHDR _waveHeaders[PLAY_QUEUE_SIZE]; // double buffering, we play 1 while we set up 2...
	  BOOL _bStopped;
	  BOOL _bPaused;
    int _numSampleSlicesPerHeader;
    int _currentHeader;

  };
}
#endif