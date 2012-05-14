#include "WaveSpectrumAnalyzer.h"
#define _USE_MATH_DEFINES true
#include <math.h>

using namespace FFTWave;

float FFTWave::hamming( float value, int sampleNum, int totalNumSample )
{
  return value * ( 0.54 - 0.56 * cos( 2 * M_PI * sampleNum / totalNumSample ) );
}

float FFTWave::noopCompute( float value ) { return value; };

float FFTWave::max1Compute( float value ) { return 1 -exp( -2 * value ); };


float toMagOrPower( kiss_fft_cpx frequencySample, bool isMagnitude, int numSamples )
{
  float power = ( frequencySample.r * frequencySample.r + frequencySample.i * frequencySample.i ) / ( numSamples * numSamples );
  if ( isMagnitude )
  {
    power = sqrt( power );
  }
  return power;
}

WaveSpectrumAnalyzer::~WaveSpectrumAnalyzer( )
{
  // TODO: free the buffers
  free(_fftCfg);
  free(_fftInData);
  free(_fftOutData);
}

BOOL WaveSpectrumAnalyzer::fft( int blockOffset, int channel )
{
  if ( setFFTInChunk( blockOffset, channel ) )
  {
    kiss_fft( _fftCfg , _fftInData , _fftOutData );
    switch ( _mode )
    {
    case Linear:
      updateBandsLinear( );
      break;
    case Log:
      updateBandsLog( );
      break;
    case Peak:
      _peakValue += findPeak( );
      break;
    }
    return TRUE;
  }
  return FALSE;
}

void WaveSpectrumAnalyzer::updateBandsLinear( )
{
    for ( int bandIndex = 0; bandIndex < _numBands; ++bandIndex )
    {
      float avgBandPower = 0;
      // start at one to avoid the DC component
      for ( int sampleIndex = 1; sampleIndex < _numSamplesPerBand; ++sampleIndex )
      {
        kiss_fft_cpx frequencySample = *( _fftOutData + ( bandIndex * _numSamplesPerBand ) + sampleIndex );
        float power = toMagOrPower( frequencySample, _magnitude, _numSamples );
        if ( _average )
        {
          avgBandPower += (  power / _numSamplesPerBand );
        }
        else
        {
          if ( power > avgBandPower ) avgBandPower = power;
        }
      }
      _fftOutDataPerBand[ bandIndex ] = ( avgBandPower / _peakValue );
    }
}

void WaveSpectrumAnalyzer::updateBandsLog( )
{
  // puts 0 in case we jump a band
  memset(_fftOutDataPerBand, 0, sizeof(float) * _numBands );

  int bandIndex = 0;
  float bandPower = 0;
  int numSamplesInBand = 0;
  int currentSample = 1; // avoid DC component
  int bandFrequency = _firstBandFrequence;
  int numFreqSamples = _numSamples / 2; // mirroring when FFT of reals
  float freqPerSample = ( (float)_waveLoader->getSampleRate( ) / numFreqSamples ); 
  while ( currentSample < numFreqSamples && bandIndex <_numBands )
  {
    float currentSampleFrequency = currentSample * freqPerSample;
    if ( currentSampleFrequency > bandFrequency )
    {
      // we are starting a new band
      if ( numSamplesInBand > 0 )
      {
        bandPower = bandPower / numSamplesInBand;
        _fftOutDataPerBand[ bandIndex ] = _transformValue( bandPower / _peakValue );
      }
      bandPower = 0;
      numSamplesInBand = 0;
      ++bandIndex;
      bandFrequency =  _firstBandFrequence * ( pow( 2, this->_frequencyGrowth * bandIndex ) );
    }
    else
    {
      kiss_fft_cpx frequencySample = *( _fftOutData + currentSample );
      float power = toMagOrPower( frequencySample, _magnitude, _numSamples );
      if ( _average )
      {
        bandPower+=power;
        ++numSamplesInBand;
      }
      else
      {
        if ( power > bandPower )
        {
          bandPower = power;
          numSamplesInBand = 1;
        }
      }
      ++currentSample;
    }
  }
}

float WaveSpectrumAnalyzer::findPeak( )
{
  float peak = 0;
  // start at one to avoid the DC component
  for ( int sampleIndex = 1; sampleIndex < _numSamples; ++sampleIndex )
  {
    kiss_fft_cpx frequencySample = *( _fftOutData + sampleIndex );
    float power = ( frequencySample.r * frequencySample.r + frequencySample.i * frequencySample.i ) / ( _numSamples * _numSamples );
    if ( _magnitude )
    {
      power = sqrt( power );
    }
    if ( power > peak )
    {
      peak = power;
    }
  }
  return peak;
}

void WaveSpectrumAnalyzer::findPeak( int numBlocks, int blockSeparation, int channel )
{
  RunningMode currentMode = _mode;
  _mode = Peak;
  _peakValue = 0.01f; // low but not zero so that it never is less than that.
  int i = 0;
  for ( ; i < numBlocks; ++i )
  {
    if ( !fft( i *  blockSeparation, channel ) )
    {
      break; // no more data to scan
    }
  }
  _peakValue = _peakValue * 2 / i;
  _mode = currentMode;
}

BOOL WaveSpectrumAnalyzer::setFFTInChunk( int blockOffset, int channel )
{
  LPSTR waveData;
  DWORD_PTR sizeLoaded;
  if ( _waveLoader->getBlocks( blockOffset, _numSamples, waveData, sizeLoaded ) )
  {
    for ( int i = 0; i < _numSamples; ++i )
    {
      setFFTSampleData( (*(_fftInData + i )).r, waveData, i, channel );
    }
    return TRUE;
  }
  return FALSE;
}

void WaveSpectrumAnalyzer::setFFTSampleData( kiss_fft_scalar& realValue, LPSTR & waveData, int sampleIndex, int channel )
{
  if ( CHANNEL_AVG == channel )
  {
    float value = 0;
    for ( int currentChannel = 0; currentChannel < _waveLoader->getChannels( ); ++currentChannel )
    {
      value += getSampleData( waveData, sampleIndex, currentChannel );
    }
    realValue = ( value / _waveLoader->getChannels( ) );
  }
  else
  {
    realValue = getSampleData( waveData, sampleIndex, channel );
  }
  realValue = hamming( realValue, sampleIndex, _numSamples );
}

float WaveSpectrumAnalyzer::getSampleData( LPSTR & waveData, int sampleIndex, int channel )
{
  int numBytesPerSample =  _waveLoader->getBitsPerSample( ) / 8;
  int index = sampleIndex * _waveLoader->getBlockAlign( ) + channel * numBytesPerSample;
  if ( 2 == numBytesPerSample )
  {
    // we are dealing with short signed integer
    short* sampleValue = (short*)( waveData + index );
    return (float)((*sampleValue) / 32768.0);
  }
  else
  {
    return 0;
  }
}
