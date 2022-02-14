#include <iostream>
#include <string>
#include "Agent.h"

SequenceElement::SequenceElement(int dummyIndex) {

    // index is also contained in the mapping object, but stored here as well
    index = dummyIndex;
    leaving_Q = 0;

}

Agent::Agent(int dummyS, int dummyA, double dummyepsilon, double dummyeta, double dummygamma, double dummyvarsigma, 
        double dummyvartheta, int dummynu, int dummyV, bool dummyPolicyEvaluation, double dummyInitRange) {

    S = dummyS;
    A = dummyA;
    pairs = S * A;
    epsilon = dummyepsilon;
    eta = dummyeta;
    gamma = dummygamma;
    varsigma = dummyvarsigma;
    vartheta = dummyvartheta;
    nu = dummynu;
    V = dummyV;
    policyEvaluation = dummyPolicyEvaluation;
    initRange = dummyInitRange;

    nuCounter = 0;

    pairIndex = 0; // to ensure will not pass arbitrary value to previousPairIndex
    state = 0; // to ensure will not pass arbitrary value to previousState
    action = 0; // to ensure will not pass arbitrary value to previousAction

    previousState = 0;
    previousAction = 0;
    previousPairIndex = 0;

    inSet = false;
    previouslyInSet = false;

    std::uniform_real_distribution<double> randomUniformDouble;
    randomUniformDouble.param(std::uniform_real_distribution<double>::param_type(-initRange, initRange));

    // intialise VF estimates Q and R
    trueQ = (double**)malloc(sizeof(double*) * S);
    Q = (double**)malloc(sizeof(double*) * S);
    _Q = (double**)malloc(sizeof(double*) * S);
    _R = (double**)malloc(sizeof(double*) * S);
    for (int i = 0; i < S; i++) {
        trueQ[i] = (double*)malloc(sizeof(double) * A);
        Q[i] = (double*)malloc(sizeof(double) * A);
        _Q[i] = (double*)malloc(sizeof(double) * A);
        _R[i] = (double*)malloc(sizeof(double) * A);
        for (int j = 0; j < A; j++) {
            trueQ[i][j] = 0;
            _Q[i][j] = randomUniformDouble(randomEngine);
            _R[i][j] = randomUniformDouble(randomEngine);
            Q[i][j] = _Q[i][j] + _R[i][j];
        }
    }

    // intialise u and uCounter as pairs x pairs matrices
    u = (double**)malloc(sizeof(double*) * pairs);
    uCounter = (int**)malloc(sizeof(int*) * pairs);
    for (int i = 0; i < pairs; i++) {
        u[i] = (double*)malloc(sizeof(double) * pairs);
        uCounter[i] = (int*)malloc(sizeof(int) * pairs);
        for (int j = 0; j < pairs; j++) {
            u[i][j] = (double)1 / pairs / pairs;
            uCounter[i][j] = 0;
        }
    }

    actionSelector = new maxWithIndex;
    uSelector = new maxNValuesWithMatrixIndices;

    // set parameters
    randomBernoulli.param(std::bernoulli_distribution::param_type(1 - epsilon));
    randomUniformInt.param(std::uniform_int_distribution<int>::param_type(0, A - 1));

}

void Agent::genTrueAct(int dummyState) {

    previousState = state;
    previousAction = action;
    previousPairIndex = pairIndex;

    state = dummyState;

    beta = randomBernoulli(randomEngine);
    if (beta) {
        if (policyEvaluation) {
            action = 0; 
        } else {
            findMaxOverRange(actionSelector, Q[state], 0, A - 1, true);
            action = actionSelector->maxIndex;
        }
    } else {
        action = randomUniformInt(randomEngine);
    }
    pair = std::pair<int,int>(state, action);
    pairIndex = state * A + action;
    
}

void Agent::genAct(int dummyState) {

    genTrueAct(dummyState);

    // update uCounter object
    uCounter[previousPairIndex][pairIndex]++;
    nuCounter++;

    // periodically update u matrix
    if (nuCounter == nu && V > 0) {
        for (int i = 0; i < pairs; i++) {
            for (int j = 0; j < pairs; j++) {
                u[i][j] = (1 - varsigma) * u[i][j] + varsigma * (double)uCounter[i][j] / nuCounter;
            }
        }
        for (int i = 0; i < pairs; i++) {
            for (int j = 0; j < pairs; j++) {
                uCounter[i][j] = 0;
            }
        }
        nuCounter = 0;
        // find the maximum V entries
        findMaxNValuesOverMatrixFast(uSelector, u, 0, pairs, 0, pairs, V);
        // clear out sets
        sequenceMap.clear();
        sequenceRecord.clear();
        // function will use uSelector object
        constructSequences();
        // remove from set since will otherwise create error (will distort one update each nu iterations, this code can be enhanced)
        inSet = false;
    }

}

void Agent::castBackUpdate() {

    for (int l = sequenceRecord.size() - 1; l >= 0; l--) {

        int index = sequenceRecord.at(l);
        SequenceElement* tempElement = sequenceMap[index];

        double tempValue = 0;
        
        for (int i = 0; i < (int)tempElement->children.size(); i++) {
            int stateIndex = tempElement->children.at(i) / A;
            int actionIndex = tempElement->children.at(i) % A;
            
            double tempProb = tempElement->transitionProbabilities.at(i);
            tempValue += tempProb * gamma * Q[stateIndex][actionIndex];
        }

        int stateIndex = index / A;
        int actionIndex = index % A;
        
        tempValue += tempElement->leaving_Q;
        _Q[stateIndex][actionIndex] = tempValue;
        
        Q[stateIndex][actionIndex] = _Q[stateIndex][actionIndex] + _R[stateIndex][actionIndex];

    }
    
}

void Agent::castBackUpdateRecursive(int dummyIndex) {

    SequenceElement* tempElement = sequenceMap[dummyIndex];

    double tempValue = 0;
    
    for (int i = 0; i < (int)tempElement->children.size(); i++) {
        int stateIndex = tempElement->children.at(i) / A;
        int actionIndex = tempElement->children.at(i) % A;
        
        double tempProb = tempElement->transitionProbabilities.at(i);
        tempValue += tempProb * gamma * Q[stateIndex][actionIndex];
    }

    int stateIndex = dummyIndex / A;
    int actionIndex = dummyIndex % A;
    
    tempValue += tempElement->leaving_Q;
    _Q[stateIndex][actionIndex] = tempValue;
    
    Q[stateIndex][actionIndex] = _Q[stateIndex][actionIndex] + _R[stateIndex][actionIndex];

    for (int i = 0; i < (int)tempElement->parents.size(); i++) {
        castBackUpdateRecursive(tempElement->parents.at(i));
    }

}

void Agent::genTrueUpdate(double reward) {

    // only apply "standard" SARSA update, to get approximate VF for every state-action pair
    trueQ[previousState][previousAction] += eta * (reward + gamma * trueQ[state][action] - trueQ[previousState][previousAction]);

}

double Agent::genUpdate(double reward) {

    previouslyInSet = inSet;
    
    // logic ensures only updates the sequence if it has not moved to a child
    if (previouslyInSet) {
        bool checkChildren = false;
        for (int i = 0; i < (int)sequenceMap[previousPairIndex]->children.size(); i++) {
            checkChildren += sequenceMap[previousPairIndex]->children.at(i) == pairIndex;
        }
        if (!checkChildren) {
            updateSequence = true;
            inSet = false;
        } else {
            updateSequence = false;
        }
    } else {
        updateSequence = false;
    }
    if (sequenceMap.count(pairIndex)) {
        inSet = true;
    }
    
    // measure is meaningless if policyEvaluation is false
    double error = (((trueQ[previousState][previousAction] - Q[previousState][previousAction]) > 0) * 2 - 1) * pow(trueQ[previousState][previousAction] - Q[previousState][previousAction], 2); 
    
    // always update R estimate
    _R[previousState][previousAction] = (1 - eta) * _R[previousState][previousAction] + eta * reward;
    
    // "standard" SARSA update on leaving_Q
    if (previouslyInSet) {
        // update leaving_Q, whether or not still in set
        sequenceMap[previousPairIndex]->leaving_Q += eta * (gamma * Q[state][action] * updateSequence - sequenceMap[previousPairIndex]->leaving_Q);
        // add to sequenceRecord the most recent element in set
        sequenceRecord.push_back(previousPairIndex);
    }

    if (updateSequence) {

        // recursively work through all parents to cast back update
        castBackUpdateRecursive(previousPairIndex);
        sequenceRecord.clear();
        
    } else if (!previouslyInSet) {

        // "standard" update
        
        // only applies if outside set
        _Q[previousState][previousAction] += eta * (gamma * Q[state][action] - _Q[previousState][previousAction]);
        // merge two estimates
        Q[previousState][previousAction] = _R[previousState][previousAction] + _Q[previousState][previousAction];
        
    }

    return error;

}

void Agent::print() {

    printf("\n");

    int maxWidth = 10;

    printf("Q\n");
    std::cout << getMatrixString(Q, S, std::min(A, maxWidth));
    printf("_Q\n");
    std::cout << getMatrixString(_Q, S, std::min(A, maxWidth)); 
    printf("_R\n");
    std::cout << getMatrixString(_R, S, std::min(A, maxWidth)); 
    printf("u\n");
    std::cout << getMatrixString(u, pairs, std::min(pairs, maxWidth)); 
    printf("uCounter\n");
    std::cout << getMatrixString(uCounter, pairs, std::min(pairs, maxWidth)); 

    printf("pairs\n");
    for (int i = 0; i < (int)uSelector->maxNValues.size(); i++) {
        printf("first pair: %d (s: %d, a: %d), second pair: %d (s: %d, a: %d) [probability: %f]\n", 
            uSelector->maxIndicesx.at(i), uSelector->maxIndicesx.at(i) / A, uSelector->maxIndicesx.at(i) % A,
            uSelector->maxIndicesy.at(i), uSelector->maxIndicesy.at(i) / A, uSelector->maxIndicesy.at(i) % A,
            uSelector->maxNValues.at(i));
    }
    printf("\n");

    printf("set\n");
    std::map<int, SequenceElement*>::iterator it;  
    for (it = sequenceMap.begin(); it != sequenceMap.end(); it++) {
        std::cout << it->first << "; leaving_Q: " << it->second->leaving_Q << "; children: "; 
        for (int i = 0; i < (int)it->second->children.size(); i++) {
            std::cout << it->second->children.at(i); 
            if (i < (int)it->second->children.size() - 1) std::cout << ", ";
        } 
        std::cout << "; probabilities: "; 
        for (int i = 0; i < (int)it->second->transitionProbabilities.size(); i++) {
            std::cout << it->second->transitionProbabilities.at(i); 
            if (i < (int)it->second->transitionProbabilities.size() - 1) std::cout << ", ";
        } 
        std::cout << "; parents: "; 
        for (int i = 0; i < (int)it->second->parents.size(); i++) {
            std::cout << it->second->parents.at(i); 
            if (i < (int)it->second->parents.size() - 1) std::cout << ", ";
        } 
        std::cout << std::endl;
    }
    printf("\n");

}

void Agent::constructSequences() {

    for (int i = 0; i < (int)uSelector->maxNValues.size(); i++) {

        // get first and second indices (indices of the two pairs)
        int firstIndex = uSelector->maxIndicesx.at(i);
        int secondIndex = uSelector->maxIndicesy.at(i);
        
        bool doLinkage = false;

        // if both already in set, need to check for cycles, otherwise just add
        if (sequenceMap.count(firstIndex) && sequenceMap.count(secondIndex)) {
            
            // will scan through every path towards children to look for a cycle
            if (!checkPairs(secondIndex, firstIndex)) {
                doLinkage = true;
            }

        }

        if (!sequenceMap.count(firstIndex) || !sequenceMap.count(secondIndex)) {
        
            if (!sequenceMap.count(firstIndex)) {
                SequenceElement* tempSequenceElement = new SequenceElement(firstIndex);
                sequenceMap.insert(std::pair<int, SequenceElement*>(firstIndex, tempSequenceElement));
            }

            if (!sequenceMap.count(secondIndex)) {
                SequenceElement* tempSequenceElement = new SequenceElement(secondIndex);
                sequenceMap.insert(std::pair<int, SequenceElement*>(secondIndex, tempSequenceElement));
            }

            doLinkage = true;
        
        }    
        
        if (doLinkage) {
            
            // create children and parent targets
            sequenceMap[firstIndex]->children.push_back(secondIndex);
            // calculate marginal probability for transition probability
            double numerator = uSelector->maxNValues.at(i);
            double denominator = 0;
            for (int j = 0; j < pairs; j++) {
                if (j != firstIndex) { 
                    denominator += u[firstIndex][j]; 
                }
            }
            double tempTransitionProbability;
            if (denominator > 0) tempTransitionProbability = numerator / denominator;
            else tempTransitionProbability = (double)1 / pairs; // in absence of any data, revert to prior, which is uniform across all pairs
            sequenceMap[firstIndex]->transitionProbabilities.push_back(tempTransitionProbability);
            sequenceMap[secondIndex]->parents.push_back(firstIndex);
        
        }

    }

    // setting leaving_Q values
    std::map<int, SequenceElement*>::iterator it;  
    for (it = sequenceMap.begin(); it != sequenceMap.end(); it++) {
        double numerator = _Q[it->first / A][it->first % A];
        for (int i = 0; i < (int)it->second->children.size(); i++) {
            int tempSecondIndex = it->second->children.at(i);
            numerator -= gamma * it->second->transitionProbabilities.at(i) * Q[tempSecondIndex / A][tempSecondIndex % A];
        }
        it->second->leaving_Q = numerator;
    }
            
}

int Agent::checkPairs(int dummyIndex, int dummyIndexToCheck) {

    for (int i = 0; i < (int)sequenceMap[dummyIndex]->children.size(); i++) {
        if (sequenceMap[dummyIndex]->children.at(i) == dummyIndexToCheck) {
            return 1;
        } else if (checkPairs(sequenceMap[dummyIndex]->children.at(i), dummyIndexToCheck)) {
            return 1;
        }
    } 
    return 0;

}