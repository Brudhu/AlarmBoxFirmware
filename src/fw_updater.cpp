#include "fw_updater.hpp"
#include "pin.hpp"

Luvitronics::FWUpdater::FWUpdater() : OTAUpdater()
{
    onProgress(FWUpdater::progressFunction);
    onEnd(FWUpdater::endFunction);
}

void Luvitronics::FWUpdater::progressFunction(unsigned int, unsigned int)
{
    Pin<13>::digitalInvert(); //SONOFF_LED = 13
}

void Luvitronics::FWUpdater::endFunction()
{
    digitalWrite(13, HIGH);
}
