#include "stdafx.h"
#include "cube.h"

namespace FFTWave
{
	Cube::Cube( float heightRatio, float width, float height, float xOffset, float yOffset )
	{
		this->_heightRatio=heightRatio; this->_height=height; this->_width=width; this->_xOffset=xOffset; this->_yOffset=yOffset;
	}

	Cube::~Cube( )
	{
	}

	void Cube::addToBatch( GLTriangleBatch& batch )
	{
    float height = _height * _heightRatio;
		M3DVector3f v1 = { this->_xOffset + 0,     0,     0 };
		M3DVector3f v2 = { this->_xOffset + _width, 0,     0 };
		M3DVector3f v3 = { this->_xOffset + _width, 0, _width };
		M3DVector3f v4 = { this->_xOffset + 0,     0, _width };
		M3DVector3f v5 = { this->_xOffset + 0,     height,     0 };
		M3DVector3f v6 = { this->_xOffset + _width, height,     0 };
		M3DVector3f v7 = { this->_xOffset + _width, height, _width };
		M3DVector3f v8 = { this->_xOffset + 0,     height, _width };
		addRecToBatch( batch, v5, v1, v2, v6 );
		addRecToBatch( batch, v5, v1, v4, v8 );
		addRecToBatch( batch, v7, v3, v2, v6 );
		addRecToBatch( batch, v7, v3, v4, v8 );
		addRecToBatch( batch, v5, v6, v7, v8 );
	}

	void Cube::setHeight( float height )
	{
	}

	void Cube::addRecToBatch( GLTriangleBatch& batch, M3DVector3f v1, M3DVector3f v2, M3DVector3f v3, M3DVector3f v4 )
	{
		// compute the normal for that side
		M3DVector3f normal;
		M3DVector3f edge1;
		M3DVector3f edge2;
		m3dSubtractVectors3( edge1, v1, v2 );
		m3dSubtractVectors3( edge2, v3, v2 );
		m3dCrossProduct3( normal, edge1, edge2 );
		M3DVector2f vTexCoords[ 3 ] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
		M3DVector3f vNorms[ 3 ];
		m3dCopyVector3( vNorms[ 0 ], normal );
		m3dCopyVector3( vNorms[ 1 ], normal );
		m3dCopyVector3( vNorms[ 2 ], normal );
		M3DVector3f triangle[3];
		m3dCopyVector3( triangle[ 0 ], v1 );
		m3dCopyVector3( triangle[ 1 ], v2 );
		m3dCopyVector3( triangle[ 2 ], v3 );
		batch.AddTriangle( triangle, vNorms, vTexCoords );
		m3dCopyVector3( triangle[ 0 ], v3 );
		m3dCopyVector3( triangle[ 1 ], v4 );
		m3dCopyVector3( triangle[ 2 ], v1 );
		batch.AddTriangle( triangle, vNorms, vTexCoords );
	}

}