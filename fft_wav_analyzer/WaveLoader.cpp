// Wave is loaded all in memory! No streaming, no nothing! Thanks for 64 bits and huge memory :)

// Largely based on http://www.codeproject.com/Articles/29676/CWave-A-Simple-C-Class-to-Manipulate-WAV-Files
// Also used http://www.sonicspot.com/guide/wavefiles.html

#include "WaveLoader.h"

using namespace FFTWave;
#include <string>

WaveLoader::WaveLoader()
{
	// Init members
	memset(&_descriptor, 0, sizeof(_WAVEDESCR));
	memset(&_format, 0, sizeof(_WAVEFORMAT));
	_lpData = NULL;
	_dwSize = 0;
  _currentBlock = 0;
  _totalOfBlocks = 0;
}

WaveLoader::~WaveLoader( )
{
	// free the data buffer
	close();
}

BOOL WaveLoader::load(const std::wstring filepath )
{
  if ( this->isLoadedFile( filepath) )
  {
    return TRUE;
  }
	BOOL bResult = FALSE;
  // Clear current Data if not null
	if ( NULL != _lpData )
  {
    return FALSE; // we already have loaded data, rather than trying to properly clean up that instance for re-use, let's yield
  }
	// Load .WAV file
  FILE* file = _tfopen(filepath.c_str( ), _T("rb"));
	if (file == NULL) return FALSE;
	// Read .WAV descriptor
	fread(&_descriptor, sizeof(_WAVEDESCR), 1, file);

	// Check for valid .WAV file
	if (strncmp((LPCSTR)_descriptor.wave, "WAVE", 4) == 0)
	{
		// Read .WAV format
		fread(&_format, sizeof(_WAVEFORMAT), 1, file);

		// Check for valid .WAV file
		if ((strncmp((LPCSTR)_format.id, "fmt", 3) == 0) && (_format.format == 1))
		{
			// Read next chunk
			BYTE id[4];
			DWORD size;
			fread(id, sizeof(BYTE), 4, file);
			fread(&size, sizeof(DWORD), 1, file);
			DWORD offset = ftell(file);

			// Read .WAV data, there can be more than one data chunk, and data
      // chunk might be interleaved with non data chunks, thus the skeep
			LPBYTE lpTemp = (LPBYTE)malloc(_descriptor.size*sizeof(BYTE));
			while (offset < _descriptor.size)
			{
				// Check for .WAV data chunk
				if (strncmp((LPCSTR)id, "data", 4) == 0)
				{
					if (_lpData == NULL)
          {
						_lpData = (LPBYTE)malloc(size*sizeof(BYTE));
          }
					else
          {
						_lpData = (LPBYTE)realloc(_lpData, (_dwSize+size)*sizeof(BYTE));
          }
					fread(_lpData+_dwSize, sizeof(BYTE), size, file);
					_dwSize += size;
				}
				else
        {
					fread(lpTemp, sizeof(BYTE), size, file);
        }

				// Read next chunk
				fread(id, sizeof(BYTE), 4, file);
				fread(&size, sizeof(DWORD), 1, file);
				offset = ftell(file);
			}
			free(lpTemp);
		}
    _totalOfBlocks = _dwSize / _format.blockAlign;
		// Close .WAV file
		fclose(file);
    this->_currentFilePath = filepath;
    bResult = TRUE;
	}
  //memset( _lpData, 66, 500000 );
  return bResult;
}

BOOL WaveLoader::getNextBlocks( int maxNumBlocks, LPSTR & dataPtr, DWORD_PTR & size )
{
  BOOL hasMoreData = getBlocks( _currentBlock, maxNumBlocks, dataPtr, size );
  if ( hasMoreData )
  {
    _currentBlock+=maxNumBlocks;
  }
  return hasMoreData;
}

BOOL WaveLoader::getBlocks( int startBlockIndex, int maxNumBlocks, LPSTR & dataPtr, DWORD_PTR & size )
{
  startBlockIndex = max(startBlockIndex, 0);
  dataPtr = (LPSTR)_lpData + ( startBlockIndex * _format.blockAlign );
  startBlockIndex+=maxNumBlocks;
  if ( startBlockIndex < _totalOfBlocks )
  {
    size = maxNumBlocks * _format.blockAlign;
    return TRUE;
  }
  else
  {
    // reaching the end of the file
    size = ( startBlockIndex - _totalOfBlocks ) * _format.blockAlign;
    return FALSE;
  }
}

void WaveLoader::reset( )
{
  _currentBlock = 0;
}

int WaveLoader::toBlockFromTime( double seconds )
{
  return  ( int ) ( seconds * (double) getSampleRate( ) );
}

void WaveLoader::close( )
{
  free(_lpData);
	_lpData = NULL;
	_dwSize = 0;
  _currentBlock = 0;
  _totalOfBlocks = 0;
}