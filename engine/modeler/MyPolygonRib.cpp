#include "MyPolygonRib.h"

MyPolygonRib::MyPolygonRib(std::vector<Vertex01*>* pVxSrc, int idx0) {
	bool shortLine = (pVxSrc->size() == 2);
	//2 points
	i0 = idx0;
	int ribsN = pVxSrc->size();
	int idx1 = (idx0 + 1) % ribsN;
	i1 = idx1;
	initLineXY(this, pVxSrc->at(idx0)->aPos, pVxSrc->at(idx1)->aPos);
	//3-rd "inner" ref point
	float* p2 = NULL;
	if (!shortLine) {
		int idx2 = (idx0 + 2) % ribsN;
		p2 = pVxSrc->at(idx2)->aPos;
	}
	//find "inner" side direction
	if (isVertical) {
		//"inner" side
		if (!shortLine) {
			if (p2[0] < x_vertical)
				xDirIn = -1;
			else
				xDirIn = 1;
		}
	}
	else if (isHorizontal) {
		//"inner" side
		if (!shortLine) {
			if (p2[1] < b_intercept)
				yDirIn = -1;
			else
				yDirIn = 1;
		}
	}
	else {
		//"inner" side
		if (!shortLine) {
			float y = a_slope * p2[0] + b_intercept;
			if (p2[1] < y)
				yDirIn = -1;
			else
				yDirIn = 1;
			float x = (p2[1] - b_intercept) / a_slope;
			if (p2[0] < x)
				xDirIn = -1;
			else
				xDirIn = 1;
		}
	}
	return;
}
