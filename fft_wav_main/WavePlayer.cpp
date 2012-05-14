#include "WavePlayer.h"

using namespace FFTWave;

WavePlayer::WavePlayer( WaveLoader* waveLoader )
{
  _waveLoader = waveLoader;
	_hWaveout = NULL;
  for ( int i = 0; i < PLAY_QUEUE_SIZE; ++i )
  {
	  memset(&_waveHeaders[i], 0, sizeof(WAVEHDR));
  }
	_bPaused = FALSE;
	_bStopped = TRUE;
  _numSampleSlicesPerHeader = 0;
  _currentHeader = 0;
}

WavePlayer::~WavePlayer( ) {}

BOOL WavePlayer::open( )
{
	BOOL bResult = TRUE;

	// Open output device
	SHORT format = WAVE_FORMAT_PCM;
  SHORT blockAlign = _waveLoader->getBlockAlign( );
	WAVEFORMATEX wfex;
	wfex.wFormatTag = format;
  wfex.nChannels = _waveLoader->getChannels( );
  wfex.nSamplesPerSec = _waveLoader->getSampleRate( );
  wfex.nAvgBytesPerSec = blockAlign * _waveLoader->getSampleRate( );
	wfex.nBlockAlign = blockAlign;
  wfex.wBitsPerSample = _waveLoader->getBitsPerSample( );
	wfex.cbSize = 0;
	if (waveOutOpen(&_hWaveout, WAVE_MAPPER, &wfex, (DWORD_PTR)&WavePlayer::waveOut_Proc_Streaming, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		_hWaveout = NULL;
		bResult = FALSE;
	}
	_bPaused = FALSE;
	_bStopped = TRUE;

	return bResult;
}

BOOL WavePlayer::close()
{
	BOOL bResult = TRUE;

	// Stop playback
	if (stop())
	{
		// Close output device
		if (waveOutClose(_hWaveout) != MMSYSERR_NOERROR)
		{
			_hWaveout = NULL;
			bResult = FALSE;
		}
	}

	return bResult;
}


BOOL WavePlayer::play( )
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (isValid() && (_bStopped || _bPaused))
	{
		if (_bPaused)
		{
			// Continue playback
			if (waveOutRestart(_hWaveout) != MMSYSERR_NOERROR)
				bResult = FALSE;
		}
		else
		{
      // Calculate Header size for 1 header every 1/30 seconds
      _numSampleSlicesPerHeader = ( _waveLoader->getSampleRate( ) / FREQUENCY ) ;
			// Start playback

      for ( int i = 0; i < PLAY_QUEUE_SIZE; ++i )
      {
          prepareAndQueueNextHeader( i, TRUE );
      }
		}
		_bPaused = FALSE;
		_bStopped = FALSE;
	}

	return bResult;
}

BOOL WavePlayer::prepareAndQueueNextHeader( int headerNum, BOOL init )
{
  // Relase sound info, no effect if header not prepared
  MMRESULT result;
  
  if ( !init && 
       ( result = waveOutUnprepareHeader(_hWaveout, &_waveHeaders[headerNum], sizeof(WAVEHDR)) )!= MMSYSERR_NOERROR)
  {
			return FALSE;
  }
  
  // check overflow here
  BOOL hasMoreData = _waveLoader->getNextBlocks( _numSampleSlicesPerHeader, _waveHeaders[ headerNum ].lpData, _waveHeaders[ headerNum ].dwBufferLength );
  _waveHeaders[ headerNum ].dwUser = headerNum;
  result = waveOutPrepareHeader(_hWaveout, &_waveHeaders[headerNum], sizeof(WAVEHDR));
	if ( result != MMSYSERR_NOERROR )
  {
		return FALSE;
  }
  result = waveOutWrite(_hWaveout, &_waveHeaders[headerNum], sizeof(WAVEHDR));
	if ( result != MMSYSERR_NOERROR )
  {
		return FALSE;
  }
  return TRUE;
}

BOOL WavePlayer::stop()
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (isValid() && (!_bStopped))
	{
		// Stop playback
		_bStopped = TRUE;
		if (waveOutReset(_hWaveout) != MMSYSERR_NOERROR)
			bResult = FALSE;
		else
		{
      // TODO unprepare all the headers
			if (waveOutUnprepareHeader(_hWaveout, &_waveHeaders[0], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
      {
				bResult = FALSE;
      }
		}
	}

	return bResult;
}

BOOL WavePlayer::pause()
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (isValid() && (!_bPaused) && (!_bStopped))
	{
		// Pause playback
		_bPaused = TRUE;
		if (waveOutPause(_hWaveout) != MMSYSERR_NOERROR)
			bResult = FALSE;
	}

	return bResult;
}

void WavePlayer::waveOut_Proc_Streaming(HWAVEOUT hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	// Get current object
	WavePlayer* pWave = (WavePlayer*)(dwInstance);
  //cout << "proc" << uMsg << endl;
	// Check for playback finished
	if ( (uMsg == WOM_DONE) )
	{
    //MMRESULT result = waveOutUnprepareHeader(pWave->m_hWaveout, (LPWAVEHDR )dwParam1 , sizeof(WAVEHDR) );
    if ( pWave->prepareAndQueueNextHeader( ( (LPWAVEHDR)dwParam1 )->dwUser ) )
    {
      ++(pWave->_currentHeader);
    }
	}

}