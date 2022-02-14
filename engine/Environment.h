#include <stdlib.h>  
#include <random>
#include "Functions.h"

// ENVIRONMENT HEADER

class Environment {

private:

    int X;
    int Y;
    int S;
    int A;
    int F;
    double zeta;
    int RNS;
    double noiseRange;

    double** R;
    int** determP;

    std::bernoulli_distribution randomBernoulli;
    std::uniform_int_distribution<int> randomUniformInt;
    std::uniform_real_distribution<double> randomUniformDouble;
    std::default_random_engine randomEngine;

public:

    int state;
    double reward;
    int f; // counter for frequency of reset
    bool looping;

    Environment(int, int, double, double, int);
    Environment() {}

    void genEnvResp(int);
    void resetEnv();

};