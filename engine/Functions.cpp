#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include "Functions.h"

// FIND MAX (starts at minRange, ends at maxRange (i.e. INCLUSIVE))
void findMaxOverRange(maxWithIndex* dummyStruct, double* vec, int minRange, int maxRange, bool forSARSA) {
	double max; 
	int maxIndex; 
	if (forSARSA) {
		max = vec[minRange];
		maxIndex = minRange;
	} else {
		max = -1;
		maxIndex = -1;
	}
	for (int i = minRange; i <= maxRange; i++) {
		if (vec[i] > max) {
			max = vec[i];
			maxIndex = i;
		}
	}
	dummyStruct->max = max;
	dummyStruct->maxIndex = maxIndex;
}

// GET LARGEST N VALUES AND INDICES
std::vector<idx_pair> getLargest(std::vector<double> const& data, int N) {

	std::priority_queue<double, std::vector<double>, std::greater<double>> pq;
	for (auto d : data) {
		if (pq.size() >= N && pq.top() < d) {
			pq.push(d);
			pq.pop();
		} else if (pq.size() < N) {
			pq.push(d);
		}
	}
	double N_element = pq.top();
	std::vector<idx_pair> result;
	// add strictly greater first
	for (int i = 0; i < data.size(); i++) {
		if (data[i] > N_element) {
			result.push_back({data[i], i});
		}
	}
	// then add amount of = to ensures minimum number of elements equal to minimum are included
	int i = 0;
	while ((int)result.size() < N) {
		if (data[i] == N_element && (int)result.size() < N) {
			result.push_back({data[i], i});
		}
		i++; 
	}
	return result;

}

// FIND POSITIONS FOR MAX N ELEMENTS IN MATRIX
void findMaxNValuesOverMatrixFast(struct maxNValuesWithMatrixIndices* dummyStruct, double** matrix, 
	int minRangex, int maxRangex, int minRangey, int maxRangey, int N) {

	dummyStruct->maxNValues.clear();
	dummyStruct->maxIndicesx.clear();
	dummyStruct->maxIndicesy.clear();

	std::vector<double> values;
		
	for (int i = minRangex; i < maxRangex; i++) {
		for (int j = minRangey; j < maxRangey; j++) {
			if (i != j) { values.push_back(matrix[i][j]); }
		}
	}
	
	std::vector<idx_pair> result;
	result = getLargest(values, N);

	for (int i = 0; i < (int)result.size(); i++) {
		dummyStruct->maxNValues.push_back(result.at(i).first);
		// adjust for the original removal of the diagonal
		int rawIndexx = result.at(i).second / (maxRangey - minRangey - 1);
		int rawIndexy = result.at(i).second % (maxRangey - minRangey - 1);
		if (rawIndexx <= rawIndexy) { rawIndexy++; }
		rawIndexx += minRangex;
		rawIndexy += minRangey;
		dummyStruct->maxIndicesx.push_back(rawIndexx);
		dummyStruct->maxIndicesy.push_back(rawIndexy);
	}

}

// FIND POSITIONS FOR MAX N ELEMENTS IN MATRIX
void findMaxNValuesOverMatrix(struct maxNValuesWithMatrixIndices* dummyStruct, double** matrix, 
	int minRangex, int maxRangex, int minRangey, int maxRangey, int N) {

	dummyStruct->maxNValues.clear();
	dummyStruct->maxIndicesx.clear();
	dummyStruct->maxIndicesy.clear();

	for (int i = minRangex; i < maxRangex; i++) {
		for (int j = minRangey; j < maxRangey; j++) {
			if (i != j) {
				if ((int)dummyStruct->maxNValues.size() < N) {
					dummyStruct->maxNValues.push_back(matrix[i][j]);
					dummyStruct->maxIndicesx.push_back(i);
					dummyStruct->maxIndicesy.push_back(j);
				} else {
					double minValue;
					double minIndex;
					if ((int)dummyStruct->maxNValues.size() > 0) {
						minValue = dummyStruct->maxNValues.at(0);
						minIndex = 0;
						for (int k = 1; k < (int)dummyStruct->maxNValues.size(); k++) {
							if (dummyStruct->maxNValues.at(k) < minValue) {
								minValue = dummyStruct->maxNValues.at(k);
								minIndex = k;
							}
						}	
						if (matrix[i][j] > minValue) {
							dummyStruct->maxNValues.erase(dummyStruct->maxNValues.begin() + minIndex);
							dummyStruct->maxIndicesx.erase(dummyStruct->maxIndicesx.begin() + minIndex);
							dummyStruct->maxIndicesy.erase(dummyStruct->maxIndicesy.begin() + minIndex);
							dummyStruct->maxNValues.push_back(matrix[i][j]);
							dummyStruct->maxIndicesx.push_back(i);
							dummyStruct->maxIndicesy.push_back(j);
						}
					}
				}
			}
		}
	}

}

std::string getMatrixString(double** matrix, int X, int Y) {
	
	std::string toReturn;
	for (int i = 0; i < X; i++) {
		toReturn += std::to_string(i) + "\t";
        for (int j = 0; j < Y; j++) {
            toReturn += std::to_string(matrix[i][j]).substr(0,6) + "\t";
        }
        toReturn += "\n";
    }
    toReturn += "\n";
	return toReturn;

}

std::string getMatrixString(int** matrix, int X, int Y) {
	
	std::string toReturn;
	for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            toReturn += std::to_string(matrix[i][j]).substr(0,6) + "\t";
        }
        toReturn += "\n";
    }
    toReturn += "\n";
	return toReturn;

}

void createCSV(std::vector<std::vector<double>> dummyResults, std::string dummyFileLocation, std::string header) {

	std::ofstream resultsFile(dummyFileLocation, std::ios::app);
    
	// construct header
	resultsFile << header + "\n";
	// send results to CSV file
	for (int i = 0; i < (int)dummyResults.size(); i++) {
		resultsFile << std::to_string(i + 1) << ",";
		for (int j = 0; j < (int)dummyResults.at(i).size(); j++) {
			resultsFile << dummyResults.at(i).at(j) << ",";
		}
		resultsFile << "\n";
	}
	resultsFile.close();

}