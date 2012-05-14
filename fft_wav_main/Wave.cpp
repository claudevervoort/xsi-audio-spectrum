#include "StdAfx.h"
#include "Wave.h"
#include "math.h"
#include <iostream>
using namespace std;


CWave::CWave(void)
{
	// Init members
	memset(&m_Descriptor, 0, sizeof(_WAVEDESCR));
	memset(&m_Format, 0, sizeof(_WAVEFORMAT));
	m_lpData = NULL;
	m_dwSize = 0;
	m_hWaveout = NULL;
	memset(&m_WaveHeader, 0, sizeof(WAVEHDR));
  for ( int i = 0; i < NUM_BLOCKS; ++i )
  {
	  memset(&m_WaveHeaders[i], 0, sizeof(WAVEHDR));
  }
	m_bPaused = FALSE;
	m_bStopped = TRUE;
  m_lastHeaderIndex = 0;
}

CWave::~CWave(void)
{
	// Close output device
	Close();
}

BOOL CWave::Open(SHORT channels, DWORD sampleRate, SHORT bitsPerSample)
{
	BOOL bResult = TRUE;

	// Open output device
	SHORT format = WAVE_FORMAT_PCM;
	SHORT blockAlign = channels << 1;
	WAVEFORMATEX wfex;
	wfex.wFormatTag = format;
	wfex.nChannels = channels;
	wfex.nSamplesPerSec = sampleRate;
	wfex.nAvgBytesPerSec = blockAlign * sampleRate;
	wfex.nBlockAlign = blockAlign;
	wfex.wBitsPerSample = bitsPerSample;
	wfex.cbSize = 0;
	if (waveOutOpen(&m_hWaveout, WAVE_MAPPER, &wfex, (DWORD_PTR)&CWave::WaveOut_Proc_Streaming, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		m_hWaveout = NULL;
		bResult = FALSE;
	}
	m_bPaused = FALSE;
	m_bStopped = TRUE;

	return bResult;
}

BOOL CWave::Close()
{
	BOOL bResult = TRUE;

	// Stop playback
	if (Stop())
	{
		// Close output device
		if (waveOutClose(m_hWaveout) != MMSYSERR_NOERROR)
		{
			m_hWaveout = NULL;
			bResult = FALSE;
		}
	}

	// Check for valid sound data
	if (IsValid())
	{
		// Clear sound data buffer
		free(m_lpData);
		m_lpData = NULL;
		m_dwSize = 0;
	}

	return bResult;
}

BOOL CWave::Load(LPTSTR lpszFilePath)
{
	BOOL bResult = FALSE;

	// Close output device
	Close();

	// Load .WAV file
	FILE* file = _tfopen(lpszFilePath, _T("rb"));
	if (file != NULL)
	{
		// Read .WAV descriptor
		fread(&m_Descriptor, sizeof(_WAVEDESCR), 1, file);

		// Check for valid .WAV file
		if (strncmp((LPCSTR)m_Descriptor.wave, "WAVE", 4) == 0)
		{
			// Read .WAV format
			fread(&m_Format, sizeof(_WAVEFORMAT), 1, file);

			// Check for valid .WAV file
			if ((strncmp((LPCSTR)m_Format.id, "fmt", 3) == 0) && (m_Format.format == 1))
			{
				// Read next chunk
				BYTE id[4];
				DWORD size;
				fread(id, sizeof(BYTE), 4, file);
				fread(&size, sizeof(DWORD), 1, file);
				DWORD offset = ftell(file);

				// Read .WAV data
				LPBYTE lpTemp = (LPBYTE)malloc(m_Descriptor.size*sizeof(BYTE));
				while (offset < m_Descriptor.size)
				{
					// Check for .WAV data chunk
					if (strncmp((LPCSTR)id, "data", 4) == 0)
					{
						if (m_lpData == NULL)
							m_lpData = (LPBYTE)malloc(size*sizeof(BYTE));
						else
							m_lpData = (LPBYTE)realloc(m_lpData, (m_dwSize+size)*sizeof(BYTE));
						fread(m_lpData+m_dwSize, sizeof(BYTE), size, file);
						m_dwSize += size;
					}
					else
						fread(lpTemp, sizeof(BYTE), size, file);

					// Read next chunk
					fread(id, sizeof(BYTE), 4, file);
					fread(&size, sizeof(DWORD), 1, file);
					offset = ftell(file);
				}
				free(lpTemp);

				// Open output device
				if (!Open(m_Format.channels, m_Format.sampleRate, m_Format.bitsPerSample))
				{
					m_hWaveout = NULL;
					bResult = FALSE;
				}
				else
					bResult = TRUE;
			}
		}

		// Close .WAV file
		fclose(file);
	}

	return bResult;
}

BOOL CWave::LoadForStreaming(LPTSTR lpszFilePath)
{
	BOOL bResult = FALSE;
	// Close output device
	Close();

	// Load .WAV file
	FILE* file = _tfopen(lpszFilePath, _T("rb"));
	if (file != NULL)
	{
		// Read .WAV descriptor
		fread(&m_Descriptor, sizeof(_WAVEDESCR), 1, file);

		// Check for valid .WAV file
		if (strncmp((LPCSTR)m_Descriptor.wave, "WAVE", 4) == 0)
		{
			// Read .WAV format
			fread(&m_Format, sizeof(_WAVEFORMAT), 1, file);

			// Check for valid .WAV file
			if ((strncmp((LPCSTR)m_Format.id, "fmt", 3) == 0) && (m_Format.format == 1))
			{
				// Read next chunk
				BYTE id[4];
				DWORD size;
				fread(id, sizeof(BYTE), 4, file);
				fread(&size, sizeof(DWORD), 1, file);
				DWORD offset = ftell(file);

				// Read .WAV data
				while (offset < m_Descriptor.size)
				{
					// Check for .WAV data chunk
					if (strncmp((LPCSTR)id, "data", 4) == 0)
					{
            m_dataChunks.push_back( CWaveDataChunk( file, offset, size ) );
						if (m_lpData == NULL)
							m_lpData = (LPBYTE)malloc(size*sizeof(BYTE));
						else
							m_lpData = (LPBYTE)realloc(m_lpData, (m_dwSize+size)*sizeof(BYTE));
						fread(m_lpData+m_dwSize, sizeof(BYTE), size, file);
						m_dwSize += size;
					}
					else
          {
						fseek( file, size, SEEK_CUR );
          }
					// Read next chunk
					fread(id, sizeof(BYTE), 4, file);
					fread(&size, sizeof(DWORD), 1, file);
					offset = ftell(file);
				}

				// Open output device
				if (!Open(m_Format.channels, m_Format.sampleRate, m_Format.bitsPerSample))
				{
					m_hWaveout = NULL;
					bResult = FALSE;
				}
				else
					bResult = TRUE;
			}
		}

		// Close .WAV file
		fclose(file);
	}

	return bResult;
}


BOOL CWave::Play()
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (IsValid() && (m_bStopped || m_bPaused))
	{
		if (m_bPaused)
		{
			// Continue playback
			if (waveOutRestart(m_hWaveout) != MMSYSERR_NOERROR)
				bResult = FALSE;
		}
		else
		{
			// Start playback
			m_WaveHeader.lpData = (LPSTR)m_lpData;
			m_WaveHeader.dwBufferLength = m_dwSize;
			if (waveOutPrepareHeader(m_hWaveout, &m_WaveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				bResult = FALSE;
			else
			{
				if (waveOutWrite(m_hWaveout, &m_WaveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					bResult = FALSE;
			}
		}
		m_bPaused = FALSE;
		m_bStopped = FALSE;
	}

	return bResult;
}

BOOL CWave::PlayStream( )
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (IsValid() && (m_bStopped || m_bPaused))
	{
		if (m_bPaused)
		{
			// Continue playback
			if (waveOutRestart(m_hWaveout) != MMSYSERR_NOERROR)
				bResult = FALSE;
		}
		else
		{
      // Calculate Header size for 1 header every 1/30 seconds
      m_headerDataSize = ( (DWORD) ( m_Format.sampleRate / BLOCKS_PER_SECOND ) ) * m_Format.blockAlign;
			// Start playback

      for ( int i = 0; i < NUM_BLOCKS; ++i )
      {
          PrepareAndQueueNextHeader( i, TRUE );
      }
		}
		m_bPaused = FALSE;
		m_bStopped = FALSE;
	}

	return bResult;
}

BOOL CWave::PrepareAndQueueNextHeader( int headerNum, BOOL init )
{
  // Relase sound info, no effect if header not prepared
  MMRESULT result;
  
  if ( !init && 
       ( result = waveOutUnprepareHeader(m_hWaveout, &m_WaveHeaders[headerNum], sizeof(WAVEHDR)) )!= MMSYSERR_NOERROR)
  {
			return FALSE;
  }
  
  // check overflow here
	m_WaveHeaders[ headerNum ].lpData = (LPSTR)m_lpData + ( m_headerDataSize * m_lastHeaderIndex * sizeof(BYTE) );
	m_WaveHeaders[ headerNum ].dwBufferLength = m_headerDataSize ;
  m_WaveHeaders[ headerNum ].dwUser = headerNum;
  result = waveOutPrepareHeader(m_hWaveout, &m_WaveHeaders[headerNum], sizeof(WAVEHDR));
	if ( result != MMSYSERR_NOERROR )
  {
		return FALSE;
  }
  result = waveOutWrite(m_hWaveout, &m_WaveHeaders[headerNum], sizeof(WAVEHDR));
	if ( result != MMSYSERR_NOERROR )
  {
		return FALSE;
  }
  ++m_lastHeaderIndex; // critical section?
  return TRUE;
}

BOOL CWave::Stop()
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (IsValid() && (!m_bStopped))
	{
		// Stop playback
		m_bStopped = TRUE;
		if (waveOutReset(m_hWaveout) != MMSYSERR_NOERROR)
			bResult = FALSE;
		else
		{
			if (waveOutUnprepareHeader(m_hWaveout, &m_WaveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				bResult = FALSE;
		}
	}

	return bResult;
}

BOOL CWave::Pause()
{
	BOOL bResult = TRUE;

	// Check for valid sound data
	if (IsValid() && (!m_bPaused) && (!m_bStopped))
	{
		// Pause playback
		m_bPaused = TRUE;
		if (waveOutPause(m_hWaveout) != MMSYSERR_NOERROR)
			bResult = FALSE;
	}

	return bResult;
}

BOOL CWave::WaveOut_Proc(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	BOOL bResult = TRUE;

	// Get current object
	CWave* pWave = (CWave*)((DWORD_PTR)&dwInstance);

	// Check for playback finished
	if ((uMsg == WOM_DONE) && ((!pWave->m_bStopped) || (!pWave->m_bPaused)))
	{
		// Relase sound info
		if (waveOutUnprepareHeader(pWave->m_hWaveout, &pWave->m_WaveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			bResult = FALSE;
	}

	return bResult;
}

void CWave::WaveOut_Proc_Streaming(HWAVEOUT hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	// Get current object
	CWave* pWave = (CWave*)(dwInstance);
  //cout << "proc" << uMsg << endl;
	// Check for playback finished
	if ( (uMsg == WOM_DONE) )
	{
    //MMRESULT result = waveOutUnprepareHeader(pWave->m_hWaveout, (LPWAVEHDR )dwParam1 , sizeof(WAVEHDR) );
    pWave->PrepareAndQueueNextHeader( ( (LPWAVEHDR)dwParam1 )->dwUser );

	}

}