#ifndef SPECTRO_GL
#define SPECTRO_GL

#include "cube.h"
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>

namespace FFTWave
{

  class SpectroGL
  {
  public:

    static SpectroGL* getGLWin( int winX = 800, int winY = 600, int numRows = 1, int numCols = 72 );

    BOOL openDisplay( int *argcp, char **argv, void (*initCallbackFunc)(int value) );
    void updateRow( int rowIndex, float* normalizedHeights );
    void draw( );
    void setCallbackFunction( void (*func)(int value) );
    // for glut callback
    void changeSize( int w, int h );
    void renderScene( );
    void keyboardEvent( unsigned char key, int x, int y );

  private:

    SpectroGL( int w, int h, int numRows, int numCols ):_numRows(numRows),_numCols(numCols)
    {
      initDimensions( w, h );
      _heightRatios = (float**) malloc( numRows * sizeof(float*) );;
      //_heightRatios = (float*) malloc( numRows * numCols * sizeof(float) );
      memset( _heightRatios, 0, numRows * sizeof(float*) );
    };
    ~SpectroGL( )
    {
      //free( _heightRatios );
    };
    void initDimensions( int w, int h );
    void setupRenderContext( );
    void addCubesToBatch( );

    static const int REFRESH_MS;
    static const float TOTAL_WIDTH;

    int _winX;
    int _winY;
    int _numRows;
    int _numCols;
    float _totalHeight;
    float _aspectRatio;
    float** _heightRatios;

    GLTriangleBatch _triangleBatch;
    GLShaderManager _shaderManager;
    GLMatrixStack _modelViewMatrix;
    GLMatrixStack _projectionMatrix;
    GLFrustum _viewFrustum;
    GLGeometryTransform _transformPipeline;

    static SpectroGL* _winglInstance;

  };

};



#endif