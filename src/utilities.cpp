#include "utilities.h"

#include <cassert>

std::vector<float> linspace(float start, float stop, unsigned int num)
{
    assert(num > 0);

    std::vector<double> ret(num);
    double dx = num > 1 ? (stop-start)/(num-1) : 0;
    for (int i = 0; i < num; ++i) ret[i] = start + i*dx;

    return ret;
}

double what_time_is_it_now()
{
    timeval time;
    if (gettimeofday(&time,NULL)) return 0;
    return (double)time.tv_sec + (double)time.tv_usec * 1e-6;
}
