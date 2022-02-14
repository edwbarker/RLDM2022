#include "Environment.h"

// "GARNET" ENVIRONMENT

Environment::Environment(int dummyS, int dummyA, double dummyzeta, double dummyNoiseRange, int dummyRNS) {

    S = dummyS;
    A = dummyA;
    zeta = dummyzeta;
    noiseRange = dummyNoiseRange;
    RNS = dummyRNS;
    
    randomBernoulli.param(std::bernoulli_distribution::param_type(zeta));
    randomUniformDouble.param(std::uniform_real_distribution<double>::param_type(-noiseRange, noiseRange));

    randomEngine.seed(RNS);

    R = (double**)malloc(sizeof(double*) * S);
    determP = (int**)malloc(sizeof(int*) * S);
    for (int i = 0; i < S; i++) {
        R[i] = (double*)malloc(sizeof(double) * A);
        determP[i] = (int*)malloc(sizeof(int) * A);
    }

    randomUniformInt.param(std::uniform_int_distribution<int>::param_type(0, S - 2));
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < A; j++) {
            bool tempRewardPositiveFlag = randomBernoulli(randomEngine);
            R[i][j] = tempRewardPositiveFlag;
            determP[i][j] = randomUniformInt(randomEngine);
            if (determP[i][j] >= i) { determP[i][j]++; }
        }
    }

    state = 0; // arbitrary starting point

}

void Environment::genEnvResp(int dummyAction) {

    reward = R[state][dummyAction];
    reward += randomUniformDouble(randomEngine) / 2;
    state = determP[state][dummyAction];

}
