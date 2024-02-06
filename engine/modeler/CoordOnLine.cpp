#include "CoordOnLine.h"
#include "utils.h"

int CoordOnLine::applyShift(CoordOnLine* pCoL, float remainingShift, int anglesToo) {
	while (1) {
		if (pCoL->startVertN + 1 >= pCoL->pVerts->size())
			return 0;
		Vertex01* pV0 = pCoL->pVerts->at(pCoL->startVertN);
		Vertex01* pV1 = pCoL->pVerts->at(pCoL->startVertN + 1);
		pCoL->segmentLength = v3lengthFromTo(pV0->aPos, pV1->aPos);
		float segmentProgress = pCoL->offsetPercent * pCoL->segmentLength;
		float segmentRemains = pCoL->segmentLength - segmentProgress;
		if (segmentRemains >= remainingShift) {
			float shiftPercent = remainingShift / pCoL->segmentLength;
			pCoL->offsetPercent += shiftPercent;
			//calculate endPoint pos
			float remainingPercent = 1.0 - pCoL->offsetPercent;
			for (int i = 0; i < 3; i++)
				pCoL->pos[i] = pV0->aPos[i] * remainingPercent + pV1->aPos[i] * pCoL->offsetPercent;
			if (anglesToo > 0) {
				pCoL->eulerDg[0] = pV0->aTangent[0] * remainingPercent + pV1->aTangent[0] * pCoL->offsetPercent;//pitch
				pCoL->eulerDg[1] = pV0->aTangent[1] * remainingPercent + pV1->aTangent[1] * pCoL->offsetPercent;//yaw
			}
			return 1;
		}
		//if here - switch to next segment
		pCoL->startVertN++;
		if (pCoL->startVertN == pCoL->pVerts->size())
			return 0; //no more verts

		remainingShift -= segmentRemains;
		pCoL->offsetPercent = 0;
		pCoL->segmentLength = 0;
	}

	return 1;
}