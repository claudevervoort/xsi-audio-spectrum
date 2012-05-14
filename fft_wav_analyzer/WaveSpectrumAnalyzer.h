#ifndef FFTW_WAVE_ANALYZER
#define FFTW_WAVE_ANALYZER

#include "kiss_fft.h"
#include "WaveLoader.h"
#include <math.h>

namespace FFTWave
{
  const int CHANNEL_AVG = -1; //Average of Left and Right Channel

  float hamming( float value, int index, int totalSample ); //hamming function to avoid 

  float noopCompute( float value ) ; // does nothing, used as post processing for values

  float max1Compute( float value ) ; // 1 - e( -2x ): never more than 1 or less than 0

  class WaveSpectrumAnalyzer
  {
  public:
    enum RunningMode
    {
      Linear, //Linear frequency bands
      Log, //log frequency bands
      Peak // running to find the peak so that values can be normalized relative to peak
    };

    WaveSpectrumAnalyzer( WaveLoader* waveLoader )
    {
      _waveLoader = waveLoader;
      _lastBlock = -1; // no block loaded yet
      _lastChannel = -1;
      _fftOutDataPerBand = NULL;
    }

    ~WaveSpectrumAnalyzer( );

    // set the analyzer to compute linear frequency bands (each band represents the same frequency range)
    void setLinear(  int power2numSamples = 10 /*1024 samples*/, 
                     int power2numBands = 8 /*64 bands*/, 
                     bool average = false /* see setLog for explanation */
                  )
    {
      _mode = Linear;
      _numSamplesPerBand = 1 << (power2numSamples - power2numBands - 1 /*ignore FFT symmetry*/);
      init( 1<<power2numSamples, 1<<power2numBands, average );
    }

    // set the analyzer to compute log based bands (which is how we 'hear' frequencies)
    // this needs more FFT samples as bands double in frequency size on each step (55hz-110hz-220hz-440hz-880hz...)
    // without enough bands we cannot capture enough info in the lower frequencies
    void setLog( int power2numSamples = 13 /*many samples needed when LOG to have at least one sample per band*/, 
                 int frequencyStart = 55 /* start frequency */, 
                 int numberOfBands = 72 , 
                 float frequencyGrowth = .125, /* each step is 1.125 of the previous one*/
                 bool average = false /* take the average of the samples in the band, if false, will take the highest value*/
                 )
    {
      _mode = Log;
      if ( _firstBandFrequence != frequencyStart || _frequencyGrowth != frequencyGrowth )
      {
        _firstBandFrequence = frequencyStart;
        _frequencyGrowth = frequencyGrowth;
        _lastBlock = -1;
      }
      init( 1<<power2numSamples, numberOfBands, average );
    }

    // post processing function, by default noop
    void setTransformFinalValueFunction(  float ( *ptFunc )( float ) )
    {
      _transformValue = ptFunc;
    }

    /*
     * Finds the peak level within the numBlockScanned and use that to scale the values
     * Block Separation means the number of samples in between scans (so 4 numBlockScanned
     * and 5 blockSeparation means scan occurs at offset 0, 5, 10, 15)
     */
    void findPeak( int numBlockScanned, int blockSeparation, int channel );
    /*
     * If true, the magnitude is returned, otherwise it is the power (i.e. magnitude squared)
     */
    void setUseMagnitude( bool magnitude ) 
    { 
      if ( _magnitude != magnitude )
      {
        _magnitude = magnitude;
        _lastBlock = -1;
      }
    };

    int getNumberOfBands( ) const
    {
      return _numBands;
    };

    /*
     * return a NULL pointer if there is no data to process the FFT 
     * (like when reaching EOF of the Wave file)
     */
    float* getFFTBands( int blockOffset, int channel ) 
    {
      if ( ( ( blockOffset == _lastBlock ) && ( channel = _lastChannel ) ) 
           || fft( blockOffset, channel ) )
      {
        _lastBlock = blockOffset; // multiple requests for the same block will just reuse the last computation
        _lastChannel = channel;
        return _fftOutDataPerBand;
      }
      return NULL;
    };

    /* 
     * Returns the previous bands computation, NULL if not yet initialized
     */
    float* getFFTBands( )
    {
      return _fftOutDataPerBand;
    }

  private:
    void init( int numSamples, int numBands, bool average )
    {
      if ( numSamples != _numSamples || numBands != _numBands || average != _average )
      {
        _numSamples = numSamples;
        _numBands = numBands;
        _magnitude = true;
        _average = average;
        _peakValue = 1;
        _fftCfg = kiss_fft_alloc( _numSamples, false ,0 ,0 );
        _fftInData  = (kiss_fft_cpx*) malloc( _numSamples * sizeof(kiss_fft_cpx) );
        _fftOutData = (kiss_fft_cpx*) malloc( _numSamples * sizeof(kiss_fft_cpx) );
        _fftOutDataPerBand = (float*) malloc( _numBands * sizeof(float) );
        _lastBlock = -1; // we are potentially changing the settings, so we should not reuse any existing FFT
        setTransformFinalValueFunction( &noopCompute );
      }
    };

    kiss_fft_cpx* getFFTResults( ) { return _fftOutData; };

    /*
     * True if there was enough data left to feed the required size
     */
    BOOL fft( int blockOffset, int channel );
    /*
     * True if there was enough data left to feed the required size
     */
    BOOL setFFTInChunk( int blockOffset, int channel );

    void setFFTSampleData( kiss_fft_scalar& realValue, LPSTR & waveData, int sampleIndex, int channel );
    /*
     * Use the result of the FFT to fill the bands result using a linear distribution
     */
    void updateBandsLinear( );
    /*
     * Scan the result of the FFT to find the frequency peak
     */
    float findPeak( );
    /*
     * Use the result of the FFT to fill the bands result using a log frequency approach
     * where each band covers twice as much frequency as the one before
     */
    void updateBandsLog( );
    /*
     * Get Sample Data at a given location for a given channel
     */
    float getSampleData( LPSTR & waveData, int sampleIndex, int channel );


    WaveLoader* _waveLoader;
    kiss_fft_cfg _fftCfg;
    kiss_fft_cpx* _fftInData; //samples in time space
    kiss_fft_cpx* _fftOutData; //samples in frequency space
    float* _fftOutDataPerBand; //bands computed based on fft results (either average or max of frequency samples in the band)
    float _peakValue; // for normalization
    int _numSamples;
    int _numBands;
    int _numSamplesPerBand; // linear only, in log it differs per band
    int _firstBandFrequence; // in log mode, the 1st band frequence threshold
    float _frequencyGrowth; // 1 -> 1 octave
    bool _magnitude; // is this magnitude or power (i.e. magnitude squared)
    bool _average; // average or max of frequency value in band
    RunningMode _mode;
    int _lastBlock; // we retain the last block played so that we do not need to compute FFT in case this is the same as the last FFT
    int _lastChannel; // last channel for which the FFT was computed
    float ( *_transformValue )( float ) ;// function pointer to the post processing band value
  };
};

#endif