#include "Line3D.h"
#include "utils.h"

int Line3D::fitBox(Line3D* pRay00, Gabarites* pWorldBox) {
	for (int dimN = 0; dimN < 3; dimN++) { //dim - dmention
		LineXY ray;
		MyPolygon rect;
		float rectDims[2];
		float rectXY[2];
		int fillPositionN = 0;
		for (int i = 0; i < 3; i++) {
			if (i == dimN) {
				ray.p0[2] = pRay00->p0[i];
				ray.p1[2] = pRay00->p1[i];
			}
			else {
				ray.p0[fillPositionN] = pRay00->p0[i];
				ray.p1[fillPositionN] = pRay00->p1[i];
				rectDims[fillPositionN] = pWorldBox->bbRad[i] * 2;
				rectXY[fillPositionN] = pWorldBox->bbMid[i];
				fillPositionN++;
			}
		}
		LineXY::calculateLine(&ray);
		MyPolygon::setRectangle(&rect, rectDims[0], rectDims[1], rectXY[0], rectXY[1]);
		//clip ray by polygpn
		if (clipByRect(&ray, &rect) == 0)
			return 0; //doesn't fit at all

		//update pRay00
		fillPositionN = 0;
		for (int i = 0; i < 3; i++) {
			if (i == dimN) {
				pRay00->p0[i] = ray.p0[2];
				pRay00->p1[i] = ray.p1[2];
			}
			else {
				pRay00->p0[i] = ray.p0[fillPositionN];
				pRay00->p1[i] = ray.p1[fillPositionN];
				fillPositionN++;
			}
		}
	}
	return 1;
}
int Line3D::clipByRect(LineXY* pRay, MyPolygon* pRect) {
	for (int ribN = 0; ribN < 4; ribN++) {
		MyPolygonRib* pRib = pRect->ribs.at(ribN);
		bool p0in = MyPolygon::correctSide(pRay->p0, pRib);
		bool p1in = MyPolygon::correctSide(pRay->p1, pRib);
		if (!p0in && !p1in)
			return 0; //out of world box
		else if (p0in && p1in)
			continue; //to next rib
		//find lines intersection
		float crossPoint[4];
		//if (LineXY::lineSegmentsIntersectionXY(crossPoint, pRay, pRib) == 0)
		if (LineXY::lineSegmentsIntersectionXY(crossPoint, pRay, pRib) == 0)
				continue; //no intersection
		//calculate z-component
		float k = v3lengthFromToXY(pRay->p0, crossPoint) / v3lengthFromToXY(pRay->p0, pRay->p1);
		crossPoint[2] = pRay->p0[2] + (pRay->p1[2] - pRay->p0[2]) * k;
		//replace out-ranged point
		if (p0in) //replace p1
			v3copy(pRay->p1, crossPoint);
		else //replace p0
			v3copy(pRay->p0, crossPoint);
		LineXY::calculateLine(pRay);
	}
	return 1;
}