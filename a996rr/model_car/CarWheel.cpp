#include "CarWheel.h"

int CarWheel::moveSubjCarWheel(CarWheel* pSS) {
    float wheelRotationSpeedDg = 3;
    pSS->ownSpeed.setEulerDg( wheelRotationSpeedDg,0,0);
    pSS->applySpeedsStandard(pSS);
    return 1;
}

