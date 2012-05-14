#pragma once

#include <vector>
#include "Mmsystem.h"

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

class CWaveDataChunk
{
private:
  FILE* _file;
  DWORD _offset; // where the data chunks begin (skipping chunk header)
  DWORD _size; // data chunk size 
public:
  CWaveDataChunk( FILE* file, DWORD offset, DWORD size ):_file(file), _offset(offset), _size(size) {};
  ~CWaveDataChunk( ) {};
};

#define NUM_BLOCKS 10
#define BLOCKS_PER_SECOND 30

class CWave
{
public:
	CWave(void);
	virtual ~CWave(void);

public:
	// Public methods
	BOOL Load(LPTSTR lpszFilePath);
	BOOL LoadForStreaming(LPTSTR lpszFilePath);
	BOOL Play();
	BOOL PlayStream();
	BOOL Stop();
	BOOL Pause();
	BOOL IsValid()				{return (m_lpData != NULL);}
	BOOL IsPlaying()			{return (!m_bStopped && !m_bPaused);}
	BOOL IsStopped()			{return m_bStopped;}
	BOOL IsPaused()				{return m_bPaused;}
	LPBYTE GetData()			{return m_lpData;}
	DWORD GetSize()				{return m_dwSize;}
	SHORT GetChannels()			{return m_Format.channels;}
	DWORD GetSampleRate()		{return m_Format.sampleRate;}
	SHORT GetBitsPerSample()	{return m_Format.bitsPerSample;}
  BOOL PrepareAndQueueNextHeader( int headerNum, BOOL init=FALSE );

private:
	// Private methods
	BOOL Open(SHORT channels, DWORD sampleRate, SHORT bitsPerSample);
	BOOL Close();
	BOOL static CALLBACK WaveOut_Proc(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void static CALLBACK WaveOut_Proc_Streaming(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

private:
	// Private members
	_WAVEDESCR m_Descriptor;
	_WAVEFORMAT m_Format;
	LPBYTE m_lpData;
	DWORD m_dwSize;
	HWAVEOUT m_hWaveout;
	WAVEHDR m_WaveHeader;
  WAVEHDR m_WaveHeaders[NUM_BLOCKS]; // double buffering, we play 1 while we set up 2...
	BOOL m_bStopped;
	BOOL m_bPaused;
  // for streaming, playing blocks by blocks
  FILE* file; // file being currently streamed
  std::vector<CWaveDataChunk> m_dataChunks;
  int m_lastHeaderIndex;
  DWORD m_headerDataSize;

};
