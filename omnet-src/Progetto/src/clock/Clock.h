#ifndef CLOCK_H_
#define CLOCK_H_

// This class is needed to simulate synchronization between hardware clocks

#include <chrono>

class Clock {
private:
    std::time_t drift{0};
    std::time_t drift_correction{0};

public:
    Clock();
    virtual ~Clock();

    std::time_t time_since_epoch();
    void update_drift_correction(std::time_t trusted_time);
};

#endif /* CLOCK_H_ */
