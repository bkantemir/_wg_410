#include "RailCoord.h"
#include "platform.h"

void RailCoord::toLog(std::string title, RailCoord* pRC) {
	mylog("%s: rail %d aligned=%d percent=%f\n", title.c_str(),pRC->railN,pRC->alignedWithRail,pRC->percentOnRail);

}

