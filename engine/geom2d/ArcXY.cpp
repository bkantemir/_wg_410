#include "ArcXY.h"
#include "utils.h"

extern float PI;
extern float degrees2radians;

void ArcXY::initArc(ArcXY* pA, float centerX, float centerY, float rad, float a00, float a01, int circleDir0) {
	pA->centerPos[0] = centerX;
	pA->centerPos[1] = centerY;
	pA->a0 = a00;
	pA->a1 = a01;
	pA->radius = rad;
	if (circleDir0 == 0) //unknown
		circleDir0 = signOf(angleDgFromTo(a00, a01));
	pA->circleDir = circleDir0;
	pA->fullCircle = (abs(a01 - a00) == 360);
	/*
	//2 points positions
	float aRd = pA->a0 * degrees2radians;
	pA->p0[0] = cos(aRd)+pA->centerPos[0];
	pA->p0[1] = sin(aRd) + pA->centerPos[1];
	aRd = pA->a1 * degrees2radians;
	pA->p1[0] = cos(aRd) + pA->centerPos[0];
	pA->p1[1] = sin(aRd) + pA->centerPos[1];
	*/
}
int ArcXY::crossLine(float* vOut2d, ArcXY* pA, LineXY* pL, int* solutionPreference) {
	//solutionPreference: 0,1-pick greater Y, -1,0 - pick lower X
	/*
	m - line aSlope
	n - line bIntercept
	p - circle center x
	q - circle center y
	circle equation:
	(x-p)^2+(y-q)^2=r^2
	unpack:
	xx-xp2+pp + yy-yq2+qq - rr = 0
	substityte y by line equation (xm+n)=0
	xx-xp2+pp + (xm+n)^2-(xm+n)q2+qq - rr = 0
	unpack:
	xx-xp2+pp + xxmm+2xmn+nn-xmq2-nq2+qq - rr = 0
	combine x:
	xx + xxmm-xp2+xmn2-xmq2+pp+nn-nq2+qq - rr = 0
	xx(1+mm) + x2(-p+mn-mq) + pp+nn-nq2+qq-rr = 0
	Quadratic Equation: ax^2+bx+c=0
	a = 1+mm
	b = (-p+mn-mq)*2
	c = pp+nn-nq2+qq-rr
	check: (m2+1)x^2+2(mc?mq?p)x+(q^2?r^2+p^2?2cq+c^2)=0.
	discriminant D:
	D = (bb-4ac)
	d=sqrt(D)
	solution
	x=(-b +/- d)/2a
	*/
	float m = pL->a_slope;
	float n = pL->b_intercept;
	float p = pA->centerPos[0];
	float q = pA->centerPos[1];
	float r = pA->radius;

	float pCross1[2] = { 0,0 };
	float pCross2[2] = { 0,0 };
	if (pL->isVertical) {
		//dxLLine2CircleCenter
		float dx = abs(pL->x_vertical - p);
		if (dx > pA->radius)
			return 0; //no intersections
		else if (dx == pA->radius) {
			//1 intersection
			vOut2d[0] = pL->x_vertical;
			vOut2d[1] = q;
			return 1;
		}
		//if here - 2 intersectios
		pCross1[0] = pL->x_vertical;
		pCross2[0] = pL->x_vertical;
		//square equision
		float x = pL->x_vertical;
		float b = -q * 2;
		float c = x * x - x * p * 2 + p * p + q * q - r * r;
		float D = b * b - c * 4; // discriminant
		float d = sqrt(D);
		//x=(-b +/- d)/2a
		pCross1[1] = (-b + d) / 2;
		pCross2[1] = (-b - d) / 2;
	}
	else { //line not vertical
		float a = 1.0 + m * m;
		float b = (-p + m * n - m * q) * 2;
		float c = p * p + n * n - n * q * 2 + q * q - r * r;
		float D = b * b - a * c * 4;
		if (D < 0)
			return 0; //no solutions
		else if (D == 1) { //1 solution
			vOut2d[0] = -b/(a*2);
			vOut2d[1] = vOut2d[0] * m + n;
			return 1;
		}
		//if here - 2 solutions
		float d = sqrt(D);
		//solution: x = (-b + / -d) / 2a
		pCross1[0] = (-b + d) / (a*2);
		pCross2[0] = (-b - d) / (a * 2);
		pCross1[1] = pCross1[0] * m + n;
		pCross2[1] = pCross2[0] * m + n;
	}
	//assuming that we have 2 solutions
	int prferredSolutionN = 1;
	for(int dimN=0; dimN <2; dimN++){ //dimension N
		if (solutionPreference[dimN] == 0)
			continue;
		float sign = solutionPreference[dimN];
		if (pCross1[dimN] * sign < pCross2[dimN] * sign)
			prferredSolutionN = 2;
		break;
	}
	if (prferredSolutionN == 1)
		v2copy(vOut2d, pCross1);
	else
		v2copy(vOut2d, pCross2);

	return 2;
}
