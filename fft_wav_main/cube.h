#ifndef spectro_cube
#define spectro_cube

#include "stdafx.h"

namespace FFTWave
{
	class Cube
	{
	public:
		Cube( float heightRatio, float width, float height, float xOffset, float yOffset );
		~Cube();
		void addToBatch( GLTriangleBatch& batch );
		void setHeight( float height );
	private:
		float _heightRatio; // 0 to 1
		float _height;
		float _xOffset;
    float _yOffset;
		float _width;
		void addRecToBatch( GLTriangleBatch& batch, M3DVector3f v1, M3DVector3f v2, M3DVector3f v3, M3DVector3f v4 );
	};
}
#endif