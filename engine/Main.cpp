#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include "Agent.h"
#include "Environment.h"

// BASIC IMPLEMENTATION OF SAR-$\sigma$

// COMMENCED 25 JULY 2021
// SIGNIFICANT ADJUSTMENTS 15 OCTOBER 2021

// run process
// two arguments must be provided: (1) flag to indicate PE (as opposed to PI) (2) parameter V
int main(int argc, char** argv) {
    
    std::string projectName;
    if (argc == 1) {
        projectName = "A1";
    } else {
        std::stringstream s;
        s << argv[1];
        s >> projectName;
    }

    printf(">> SAR-$\\sigma$ C++ implementation, January 2022, RLDM 2022 submission\n\n");
            
    if (argc != 1 && argc != 5) {
        printf("Wrong number of parameters entered, requires two or none\n");
        exit(0);
    }
    
    // global parameters
    bool policyEvaluation;
    if (argc == 1) {
        policyEvaluation = false;
    } else {
        policyEvaluation = std::atoi(argv[2]);
    }
    unsigned long long U = 50000000; // number of iterations to get VF estimate
    unsigned long long T = 1000000; // number of iterations
    int RNS = 1;
    
    int intT = 100; // number of intervals at which print statement occurs 
    int E = 50; // number of independent experiments
   
    // environment parameters
    int S = 1000;
    int A = 2;
    double zeta = 0.01;
    double noiseRange = 2; // implies uniform reward noise over range [-1, 1]
    
    // agent parameters
    double epsilon = 0.01;
    double eta;
    if (policyEvaluation) {
        eta = 0.001;
    } else {
        eta = 0.01;
    }
    double gamma = 0.99;
    double varsigma = 0.1; // note, step size "diluted" by delay nu, so should be relatively large compared to eta
    double vartheta = 0.99;
    int nu = 10000;
    int V;
    if (argc == 1) {
        V = 0;
    } else {
        V = std::atoi(argv[3]);
    }
    double initRange = 2; // initialise each Q uniformly randomly over the range [-2, 2]

    bool printAgent = false;

    int interval = T / intT;

    std::vector<std::vector<double>> resultsReward;
    std::vector<std::vector<double>> resultsError;
    std::vector<std::vector<double>> resultsTime;

    for (int e = 0; e < E; e++) {

        std::vector<double> resultReward;
        std::vector<double> resultError;
        std::vector<double> resultTime;

        Agent* agent = new Agent(S, A, epsilon, eta, gamma, varsigma, vartheta, nu, V, policyEvaluation, initRange);
        Environment* env = new Environment(S, A, zeta, noiseRange, e + RNS);
        
        double averageReward = 0;
        double averageError = 0;
        long totalTime;
    
        // evaluate policy using "standard" SARSA, in order to measure error in next step

        printf(">> Training run %d (V = %d, PE = %d)\n\n", e + 1, V, policyEvaluation);
        
        if (policyEvaluation) {
            printf(">> Training separate Q values using SARSA, to evaluate approximate VF error during training\n\n");
            for (unsigned long long u = 0; u < U; u++) {
                agent->genTrueAct(env->state);
                agent->genTrueUpdate(env->reward);
                env->genEnvResp(agent->action);
                if ((u + 1) % (U / 20) == 0) {
                    printf("... %d%% ", (int)((u * 100) / U + 1)); 
                }
            }
            printf("\n\n");
        }

        totalTime = 0;
        auto timeTotal0 = std::chrono::high_resolution_clock::now();
        auto timeTotal1 = std::chrono::high_resolution_clock::now();
        
        // evaluate current agent parameter settings

        printf(">> Commencing training\n\n");
        
        for (int t = 0; t < T; t++) {

            agent->genAct(env->state);
            
            averageError += agent->genUpdate(env->reward);

            env->genEnvResp(agent->action);

            averageReward += env->reward;
            
            if ((t + 1) % interval == 0) {
                
                timeTotal1 = std::chrono::high_resolution_clock::now();
    
                // time in microseconds
                totalTime = (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(timeTotal1 - timeTotal0).count();
                
                printf("e = %d, i = %d, t = %d s = %d a = %d beta = %d average reward = %f, average error = %f, total time (ms) = %ld\n", 
                    (e + 1), (t / interval + 1), (t + 1), env->state, agent->action, agent->beta, averageReward / interval, averageError / interval, totalTime);
                if (printAgent) {
                    agent->print();
                }
                
                resultReward.push_back(averageReward / interval);
                resultError.push_back(averageError / interval);
                resultTime.push_back((double)totalTime);
                
                averageReward = 0;
                averageError = 0;

                totalTime = 0;
                timeTotal0 = std::chrono::high_resolution_clock::now();
    
            }

        }

        resultsReward.push_back(resultReward);
        resultsError.push_back(resultError);
        resultsTime.push_back(resultTime);

        printf("\n>> Finished training run\n\n");

    }

    // code for datetime string to add to save file
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    auto str = oss.str();

    printf(">> Writing results\n\n");

    std::string policyEvaluationString;
    if (policyEvaluation) {
        policyEvaluationString = "PE";
    } else {
        policyEvaluationString = "PI";
    }
    std::string resultsLocation;
    if (argc == 1) {
		// replace default results save location if required
        resultsLocation = "~/results/";
    } else {
        std::stringstream s;
        s << argv[4];
        s >> resultsLocation;
    }
    try {
        std::string resultsFile = resultsLocation + str + "_compound_test_" + projectName + "_" 
            + policyEvaluationString + "_V" + std::to_string(V) + ".csv";
        createCSV(resultsReward, resultsFile, "Trial reward");
        createCSV(resultsError, resultsFile, "Trial error");
        createCSV(resultsTime, resultsFile, "Trial time");
    } catch (...) {
        printf("Could not write to location, check folder parameter for location to write results to\n");
        exit(0);
    }
    
    return 0;

}