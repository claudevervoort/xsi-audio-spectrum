#pragma once

#ifndef FFTW_WAVE_LOADER
#define FFTW_WAVE_LOADER

#include <cstdio>
#include "StdAfx.h"
#include <string>

namespace FFTWave
{
  #pragma pack(1)
  typedef struct __WAVEDESCR
  {
	  BYTE riff[4];
	  DWORD size;
	  BYTE wave[4];

  } _WAVEDESCR, *_LPWAVEDESCR;

  typedef struct __WAVEFORMAT
  {
	  BYTE id[4];
	  DWORD size;
	  SHORT format;
	  SHORT channels;
	  DWORD sampleRate;
	  DWORD byteRate;
	  SHORT blockAlign;
	  SHORT bitsPerSample;

  } _WAVEFORMAT, *_LPWAVEFORMAT;
  #pragma pack()

  class WaveLoader
  {
    private:
      std::wstring _currentFilePath;
      FILE* _file;
      DWORD _offset; // where the data chunks begin (skipping chunk header)
      DWORD _size; // data chunk size 
	    _WAVEDESCR _descriptor;
	    _WAVEFORMAT _format;
	    LPBYTE _lpData;
	    DWORD _dwSize;
      int _currentBlock; 
      int _totalOfBlocks;

    public:
      WaveLoader( ) ;
      ~WaveLoader( );
  	  LPBYTE getData()			{return _lpData;} 
	    DWORD getSize()				{return _dwSize;}
	    SHORT getChannels()			{return _format.channels;}
	    DWORD getSampleRate()		{return _format.sampleRate;}
	    SHORT getBitsPerSample()	{return _format.bitsPerSample;}
      SHORT getBlockAlign() {return _format.blockAlign;}
      BOOL load(const std::wstring filePath);
      BOOL isLoadedFile( const std::wstring filepath ) { return filepath == this->_currentFilePath; }; //will return false if file is already loaded (loader can be used only for a single file)
      BOOL getBlocks( int startBlockIndex, int maxNumBlocks, LPSTR & dataPtr, DWORD_PTR & size );
      BOOL getNextBlocks( int maxNumBlocks, LPSTR & dataPtr, DWORD_PTR & size );
      int toBlockFromTime( double seconds );
      void reset( );
      void close( );
  };
}

#endif