#ifndef FW_UPDATER_HPP
#define FW_UPDATER_HPP

#include "ota_updater.hpp"

namespace Luvitronics
{
    class FWUpdater : public OTAUpdater
    {
    public: 
        FWUpdater();
        
        static void progressFunction(unsigned, unsigned);
        static void endFunction();
    };
}

#endif //FW_UPDATER_HPP
