#pragma once
#include "geom2d/LineXY.h"
#include "Vertex01.h"
#include <vector>

class MyPolygonRib : public LineXY
{
public:
	int i0;
	int i1;
	//direction to "inner" side
	float xDirIn = 0;
	float yDirIn = 0;
public:
	MyPolygonRib(std::vector<Vertex01*>* pVxSrc, int idx0);
};


