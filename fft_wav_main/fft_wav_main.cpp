// fft_wav_main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "spectrogl.h"
#include "WavePlayer.h"
#include "WaveSpectrumAnalyzer.h"
#include <iostream>

using namespace FFTWave;
using namespace std;

WaveLoader loader;
WaveLoader loader2;
//WaveSpectrumAnalyzer analyzer( &loader2, 10 /*1024 bins*/, 8 /*64 bands*/ );
WaveSpectrumAnalyzer analyzer( &loader2 );
WavePlayer player( &loader );
int lastBlock = -1;

void glutCallback( int value )
{
  if ( player.getCurrentHeader( ) > lastBlock )
  {
    SpectroGL* glWin = SpectroGL::getGLWin( );
    lastBlock = player.getCurrentHeader( );
    float* fftBands = analyzer.getFFTBands( player.getNumBlocksPerHeader( ) * lastBlock, CHANNEL_AVG );
    if ( NULL != fftBands )
    {
      glWin->updateRow( 0, fftBands );
    }
  }
  SpectroGL::getGLWin( )->setCallbackFunction( glutCallback );
}

void glutCallbackInit( int value )
{
//  TCHAR* waveFile = _T("d:\\dev\\cpp_sandbox\\fft_wav\\c_est_moi.wav");
  //wstring waveFile(L"d:\\dev\\cpp_sandbox\\fft_wav\\CrescendoOctave.wav" );
  wstring waveFile(L"d:\\temp\\slowly.wav" );
  loader.load( waveFile );
  loader2.load( waveFile );
  analyzer.setLog( );
  //analyzer.setUseMagnitude( true );
  //analyzer.findPeak( 1024, player.getNumBlocksPerHeader( ), CHANNEL_AVG );
  analyzer.findPeak( 1024, 1024, CHANNEL_AVG );
  analyzer.setTransformFinalValueFunction( &FFTWave::max1Compute );
  player.open( );
  player.play( );
  SpectroGL::getGLWin( )->setCallbackFunction( glutCallback );
}


int main(int argc, char* argv[])
{
  SpectroGL* glwin = SpectroGL::getGLWin( );
  if ( !glwin->openDisplay( &argc, argv, glutCallbackInit ) )
  {
    cout << "could not open display" << endl;
  }
}

