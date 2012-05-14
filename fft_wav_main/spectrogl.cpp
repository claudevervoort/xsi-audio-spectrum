// spectrogl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "spectrogl.h"

using namespace FFTWave;

SpectroGL* SpectroGL::_winglInstance = NULL;
const int SpectroGL::REFRESH_MS = 16;
const float SpectroGL::TOTAL_WIDTH = 10;

void ChangeSize( int w, int h )
{
  SpectroGL::getGLWin( )->changeSize( w, h );
};

void RenderScene( void )
{
  SpectroGL::getGLWin( )->renderScene( );
};

void KeyboardFunc( unsigned char key, int x, int y )
{
  SpectroGL::getGLWin( )->keyboardEvent( key, x, y );
};



SpectroGL* SpectroGL::getGLWin( int winX, int winY, int numRows, int numCols )
{
  if ( NULL == _winglInstance )
  {
    _winglInstance = new SpectroGL( winX, winY, numRows, numCols );
  }
  return _winglInstance;
};



void SpectroGL::setupRenderContext( )
{
	glClearColor( 0.0f,0.0f, 1.0f, 1.0f );
	_shaderManager.InitializeStockShaders( );
  _modelViewMatrix.Translate( -3.5f, 0, -9.0f );
//  _modelViewMatrix.Translate( TOTAL_WIDTH/2, _totalHeight/2, -8.0f );
  addCubesToBatch( );
}

void SpectroGL::addCubesToBatch( )
{
  float rowHeight = _totalHeight / _numRows;
  float width = TOTAL_WIDTH / _numCols;
  float colWidth = width * 0.9;
	_triangleBatch.BeginMesh( _numRows * _numCols * 8 * 10 );
  
  for ( int r = 0; r<_numRows; ++r )
  {
    for ( int c = 0; c<_numCols; ++c )
    {
      float heightRatio = 1;
      if ( _heightRatios[ r ] != NULL )
      {
        heightRatio = _heightRatios[ r ] [ c ];
      }
	    Cube cube( heightRatio, colWidth, rowHeight, c * width, r * rowHeight );
	    cube.addToBatch( _triangleBatch );
    }
  }
  _triangleBatch.End( );
}

void SpectroGL::updateRow( int rowIndex, float* normalizedHeights )
{
  _heightRatios[ rowIndex ] = normalizedHeights;
  addCubesToBatch( );
  renderScene( );
}

void SpectroGL::renderScene( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	GLfloat red[ ] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	_shaderManager.UseStockShader( GLT_SHADER_FLAT, _transformPipeline.GetModelViewProjectionMatrix( ), red );
	_triangleBatch.Draw( );
	glutSwapBuffers( );
}

void SpectroGL::keyboardEvent( unsigned char key, int x, int y )
{
  switch ( key )
  {
    case 100: //d
      _modelViewMatrix.Translate( 0.1f, 0, 0 );
      break;
    case 97: //a
      _modelViewMatrix.Translate( -0.1f, 0, 0 );
      break;
    case 119://w
      _modelViewMatrix.Translate( 0, 0.1f, 0 );
      break;
    case 115://s
      _modelViewMatrix.Translate( 0, -0.1f, 0 );
      break;
    case 122:
      _modelViewMatrix.Translate( 0, 0, 0.1f );
      break;
    case 99:
      _modelViewMatrix.Translate( 0, 0, -0.1f );
      break;
  }
  renderScene( );
}

void SpectroGL::setCallbackFunction( void (*func)(int value) )
{
  glutTimerFunc( REFRESH_MS, func, 0 );
}

void SpectroGL::changeSize( int w, int h )
{
  initDimensions( w, h );
	glViewport( 0, 0, w, h );
	_viewFrustum.SetPerspective( 45.0f, _aspectRatio, 1.0f, 100.0f );
	_projectionMatrix.LoadMatrix( _viewFrustum.GetProjectionMatrix( ) );
	_transformPipeline.SetMatrixStacks( _modelViewMatrix, _projectionMatrix );
}

void SpectroGL::initDimensions( int w, int h )
{
  _winX = w;
  _winY = h;
  _aspectRatio = (float)_winX / (float)_winY;
  _totalHeight = TOTAL_WIDTH / _aspectRatio;
}

BOOL SpectroGL::openDisplay( int *argcp, char **argv, void (*initCallbackFunc)(int value) )
{
	glutInit( argcp, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL );
	glutInitWindowSize( _winX, _winY );
	glutCreateWindow( "Triangle" );
	glutReshapeFunc( ChangeSize );
	glutDisplayFunc( RenderScene );
  glutKeyboardFunc( KeyboardFunc );
  glutTimerFunc( 16, initCallbackFunc, 0 );
	GLenum err = glewInit( );
	if ( GLEW_OK != err )
	{
		fprintf( stderr, "GLEW Error: %s\n", glewGetErrorString( err ) );
		return FALSE;
	}

	setupRenderContext( );
	glutMainLoop( );
  return TRUE;

};


