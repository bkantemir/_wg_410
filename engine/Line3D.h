#pragma once
#include "Gabarites.h"
#include "MyPolygon.h"

class Line3D
{
public:
	float p0[4] = { 0,0,0,0 };
	float p1[4] = { 0,0,0,0 };
public:
	static int fitBox(Line3D* pRay, Gabarites* pWorldBox);
	static int clipByRect(LineXY* pRay, MyPolygon* pRect);
};
