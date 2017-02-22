#ifndef DATE_TIME_HPP
#define DATE_TIME_HPP

#include "task.hpp"


namespace Luvitronics
{
    class DateTime : public Task
    {
    public: 
        DateTime();
        virtual void process() override;
        
    private:
    };
}

#endif //DATE_TIME_HPP
