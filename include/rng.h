#ifndef RNG_H
#define RNG_H

#include <functional>
#include <random>
#include <string>

class RNG {
public:
    RNG(std::pair<int, int> intRange = {0, 100},
        std::pair<float, float> floatRange = {0.0f, 1.0f},
        std::pair<double, double> doubleRange = {0.0, 1.0})
        : rng(this->rd())
        , iDist(intRange.first, intRange.second)
        , fDist(floatRange.first, floatRange.second)
        , dDist(doubleRange.first, doubleRange.second)
    {
    }

    explicit RNG(float lower, float upper)
        : RNG({(int)lower,(int)upper}, {(float)lower, (float)upper}, {(double)lower, (double)upper})
    {
    }

    //interface of RNG class
    inline bool getBool() { return this->bDist(this->rng); }
    inline float getFloat() { return this->fDist(this->rng); }
    inline double getDouble() { return this->dDist(this->rng); }
    inline int getInt() { return this->iDist(this->rng); }

    inline std::mt19937 getGenerator() const { return this->rng; }

    std::string getString(std::string::size_type length = 10) {
        static const char charset[] = "0123456789"
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static std::uniform_int_distribution<> strIntDist(0, sizeof(charset) - 2);
        static auto getCharacter = [=]() -> char { return charset[strIntDist(this->rng)]; };

        std::string ret;
        ret.reserve(length);
        std::generate_n(std::back_inserter(ret), length, getCharacter);

        return ret;
    }

    void setIntRange(int lower, int upper) { this->iDist = std::uniform_int_distribution<int>(lower,upper); }
    void setFloatRange(float lower, float upper) { this->fDist = std::uniform_real_distribution<float>(lower,upper); }
    void setDoubleRange(double lower, double upper) { this->dDist = std::uniform_real_distribution<double>(lower,upper); }

    //Returns a uniformly distributed integer in range [0,n]
    int Rand(int n) { return std::uniform_int_distribution<>(0,n)(this->rng);}

private:
    std::random_device rd;
    std::mt19937 rng;

    std::uniform_int_distribution<int> iDist;
    std::uniform_real_distribution<float> fDist;
    std::uniform_real_distribution<double> dDist;
    std::bernoulli_distribution bDist;
};

#endif // RNG_H
