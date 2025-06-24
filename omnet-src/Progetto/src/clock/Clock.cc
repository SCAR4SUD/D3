//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Clock.h"

#include <chrono>
#include <stdlib.h>

Clock::Clock()
{
    srand(std::time(0));
    // initial drift from 1 to 5 seconds
    drift = (std::time_t) ((rand()%5) + 1);
}

Clock::~Clock()
{
    // TODO Auto-generated destructor stub
}

std::time_t Clock::time_since_epoch()
{
    return std::time(0) + drift - drift_correction;
    drift *= ( (rand()%2) + ( ( (rand()%100) + 1 ) / 100 ) );
}

void Clock::update_drift_correction(std::time_t trusted_time)
{
    drift_correction = trusted_time - time_since_epoch();
}
