#include "LineXY.h"
#include "utils.h"

void LineXY::initDirXY(LineXY* pLn, float* p01) {
	float p00[3] = { 0,0 };
	initLineXY(pLn, p00, p01);
}
void LineXY::initLineXY(LineXY* pLn, float* p00, float* p01) {
	v2copy(pLn->p0, p00);
	v2copy(pLn->p1, p01);
	calculateLine(pLn);
}
void LineXY::initLineXZ(LineXY* pLn, float* p00, float* p01) {
	pLn->p0[0] = p00[0];
	pLn->p0[1] = p00[2];
	pLn->p1[0] = p01[0];
	pLn->p1[1] = p01[2];
	calculateLine(pLn);
}
void LineXY::initLineZY(LineXY* pLn, float* p00, float* p01) {
	pLn->p0[0] = p00[2];
	pLn->p0[1] = p00[1];
	pLn->p1[0] = p01[2];
	pLn->p1[1] = p01[1];
	calculateLine(pLn);
}
void LineXY::calculateLine(LineXY* pLn) {
	if (v2match(pLn->p0, pLn->p1))
		pLn->isDot = true;
	else {
		pLn->isDot = false;
		//find line equation
		if (pLn->p0[0] == pLn->p1[0]) {
			pLn->isVertical = true;
			pLn->x_vertical = pLn->p0[0];
		}
		else if (pLn->p0[1] == pLn->p1[1]) {
			pLn->isHorizontal = true;
			pLn->a_slope = 0;
			pLn->b_intercept = pLn->p0[1];
		}
		else {
			pLn->a_slope = (pLn->p1[1] - pLn->p0[1]) / (pLn->p1[0] - pLn->p0[0]);
			pLn->b_intercept = pLn->p0[1] - pLn->a_slope * pLn->p0[0];
		}
	}
}
bool LineXY::matchingLines(LineXY* pLine0, LineXY* pLine1) {
	if (!parallelLines(pLine0, pLine1))
		return false;
	if (pLine0->b_intercept != pLine1->b_intercept)
		return false;
	if (pLine0->x_vertical != pLine1->x_vertical)
		return false;
	return true;
}
bool LineXY::parallelLines(LineXY* pLine0, LineXY* pLine1) {
	if (pLine0->isVertical != pLine1->isVertical)
		return false;
	if (pLine0->a_slope != pLine1->a_slope)
		return false;
	return true;
}
int LineXY::linesIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2) {
	//returns 0 if no intersection
	if (parallelLines(pL1, pL2))
		return 0;
	//find lines intersection, assuming lines are not parallel
	float x, y;
	if (pL1->isVertical) {
		x = pL1->p0[0];
		y = pL2->a_slope * x + pL2->b_intercept;
	}
	else { //pL1 not vertical
		if (pL2->isVertical) {
			x = pL2->p0[0];
			y = pL1->a_slope * x + pL1->b_intercept;
		}
		else { //both lines are "normal"
			x = (pL2->b_intercept - pL1->b_intercept) / (pL1->a_slope - pL2->a_slope);
			y = pL1->a_slope * x + pL1->b_intercept;
		}
	}
	vOut[0] = x;
	vOut[1] = y;
	return 1;
}
int LineXY::lineSegmentsIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2) {
	//returns 0 if no intersection
	if(linesIntersectionXY(vOut,pL1, pL2) == 0)
		return 0; //lines not crossing
	if (!isPointBetween(vOut, pL1->p0, pL1->p1))
		return 0; //intersection is out of range
	return 1;
}
bool LineXY::isPointOnLine(float* p2d, LineXY* pL1) {
	if (p2d[1] != (pL1->a_slope * p2d[0] + pL1->b_intercept))
		return false; //not on line
	return true;
}
bool LineXY::isPointIn(float* p2d, LineXY* pL1) {
	if (p2d[1] != (pL1->a_slope * p2d[0] + pL1->b_intercept))
		return false; //not on line
	if (isPointBetween(p2d, pL1->p0, pL1->p1))
		return true;
	return false;
}

bool LineXY::isPointBetween(float* p2d, float* v1, float* v2) {
	for(int i=0;i<2;i++)
		if(((p2d[i] - v1[i]) * (p2d[i] - v2[i])) > 0)
		return false;
	return true;
}
float LineXY::dist_l2p(LineXY* pLn, float* p2) {
	if (pLn->isDot) {
		v3copy(pLn->closestPoint, pLn->p0);
		return dist_p2p(pLn->p0, p2);
	}
	if (isPointOnLine(p2, pLn)) {
		if (isPointBetween(p2, pLn->p0, pLn->p1)) {
			v3copy(pLn->closestPoint, p2);
			return 0;
		}
		//point is on line, but outside of segment
		float dist0 = v3lengthFromToXY(p2, pLn->p0);
		float dist1 = v3lengthFromToXY(p2, pLn->p1);
		if (dist0 <= dist1) {
			v3copy(pLn->closestPoint, pLn->p0);
			return dist0;
		}
		else {
			v3copy(pLn->closestPoint, pLn->p1);
			return dist1;
		}
	}
	//point is not on line
	LineXY perp;
	buildPerpendicular(&perp, pLn, p2);
	//find intersection
	linesIntersectionXY(pLn->closestPoint, pLn, &perp);
	if (isPointBetween(pLn->closestPoint, pLn->p0, pLn->p1))
		return v3lengthFromToXY(pLn->closestPoint,p2);
	//point is on line, but outside of segment
	float dist0 = v3lengthFromToXY(p2, pLn->p0);
	float dist1 = v3lengthFromToXY(p2, pLn->p1);
	if (dist0 <= dist1) {
		v3copy(pLn->closestPoint, pLn->p0);
		return dist0;
	}
	else {
		v3copy(pLn->closestPoint, pLn->p1);
		return dist1;
	}
}
void LineXY::buildPerpendicular(LineXY* pOut, LineXY* pLn, float* p2d) {
	pOut->isDot = false;
	if (pLn->isVertical) {
		pOut->isVertical = false;
		pOut->isHorizontal = true;
		pOut->a_slope = 0; //horizontal
		pOut->b_intercept = p2d[1];
	}
	else if (pLn->a_slope==0) { //horizontal
		pOut->isVertical = true;
		pOut->isHorizontal = false;
		pOut->x_vertical = p2d[0];
	}
	else {
		pOut->isVertical = false;
		pOut->isHorizontal = false;
		pOut->a_slope = -1.0 / pLn->a_slope;
		pOut->b_intercept = p2d[1] - p2d[0]* pOut->a_slope;
	}
	//define 2nd point
	v3copy(pOut->p0, p2d);
	v3copy(pOut->p1, p2d);
	if (pOut->isVertical) {
		pOut->p1[1] += 1;
	}
	else {
		pOut->p1[0] += 1;
		pOut->p1[1] = pOut->p1[0] * pOut->a_slope + pOut->b_intercept;
	}
}
float LineXY::dist_p2p(float* p1, float* p2) {
	return v3lengthFromToXY(p1, p2);
}
float LineXY::dist_l2l(LineXY* pL1, LineXY* pL2) {
	if (pL1->isDot) {
		v3copy(pL1->closestPoint, pL1->p0);
		return dist_l2p(pL2,pL1->p0);
	}
	if (!parallelLines(pL1, pL2)) {
		if (lineSegmentsIntersectionXY(pL1->closestPoint, pL1, pL2) > 0) {
			//lines crossing
			v3copy(pL2->closestPoint, pL1->closestPoint);
			return 0;
		}
	}
	//if here-hind closest points
	float l1closestPoint[4];
	float l2closestPoint[4];
	float bestDistance = 1000000;
	float dist = dist_l2p(pL1, pL2->p0);
	if (bestDistance > dist) {
		bestDistance = dist;
		v3copy(l1closestPoint, pL1->closestPoint);
		v3copy(l2closestPoint, pL2->p0);
	}
	if (bestDistance > 0) {
		dist = dist_l2p(pL1, pL2->p1);
		if (bestDistance > dist) {
			bestDistance = dist;
			v3copy(l1closestPoint, pL1->closestPoint);
			v3copy(l2closestPoint, pL2->p1);
		}
	}
	if (bestDistance > 0) {
		dist = dist_l2p(pL2, pL1->p0);
		if (bestDistance > dist) {
			bestDistance = dist;
			v3copy(l1closestPoint, pL1->p0);
			v3copy(l2closestPoint, pL2->closestPoint);
		}
	}
	if (bestDistance > 0) {
		dist = dist_l2p(pL2, pL1->p1);
		if (bestDistance > dist) {
			bestDistance = dist;
			v3copy(l1closestPoint, pL1->p1);
			v3copy(l2closestPoint, pL2->closestPoint);
		}
	}
	v3copy(pL1->closestPoint, l1closestPoint);
	v3copy(pL2->closestPoint, l2closestPoint);
	return bestDistance;
}
