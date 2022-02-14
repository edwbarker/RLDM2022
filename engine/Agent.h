#include <map>
#include <random>
#include "Functions.h"

// AGENT HEADER

class SequenceElement {

public:

    int index;

    std::vector<int> children;
    std::vector<int> parents;

    std::vector<double> transitionProbabilities;

    double leaving_Q;

    SequenceElement() {}
    SequenceElement(int);

};

class Agent {

private:
 
public:

    // fixed values
    int S;
    int A;
    int pairs;
    double epsilon; // greedy parameter
    double eta; // learning rate
    double gamma; // discount rate
    
    bool policyEvaluation;
    double initRange;

    // dynamic values
    int action;
    int state;
    int beta; // on policy if equals 1
    std::pair<int, int> pair;
    int pairIndex;
    int previousPairIndex;

    // compound sequence parameters
    int nu; // iterations before update
    int V; // number of pairs
    double varsigma; // learning rate
    double vartheta; // tolerance threshold

    // dynamic values
    int nuCounter;
    int tempCounter = 0; // DELETE (FOR TEMP TEST)!

    // SARSA objects
    // for estimating error only
    double** trueQ;
    // standard SARSA objects
    double** Q;
    double** _Q;
    double** _R; 
    int previousState;
    int previousAction;
    maxWithIndex* actionSelector;
    std::default_random_engine randomEngine;
    std::bernoulli_distribution randomBernoulli;
    std::uniform_int_distribution<int> randomUniformInt;

    // compound sequence specific objects
    double** u; // for storing probabilities (of pairs-of-pairs)
    int** uCounter; // for storing counts
    bool inSet; // indicates in set of pairs
    bool previouslyInSet;
    bool updateSequence;
    maxNValuesWithMatrixIndices* uSelector; 
    std::map<int, SequenceElement*> sequenceMap; // to store sequence related information
    std::vector<int> sequenceRecord;
    
    Agent(int, int, double, double, double, double, double, int, int, bool, double);
    Agent() {}

    void genTrueAct(int);
    void genAct(int);
    void genTrueUpdate(double); // bool indicates whether to update the actual values Q
    double genUpdate(double); // bool indicates whether to update the actual values Q
    void print();
    
    // specialised functions
    void castBackUpdate();
    void castBackUpdateRecursive(int);
    void constructSequences();
    int checkPairs(int, int);
    
};