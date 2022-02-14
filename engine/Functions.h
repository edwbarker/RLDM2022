#pragma once
#include <stdio.h>
#include <random>

typedef std::pair<double, int> idx_pair;

struct maxWithIndex {
	double max;
	int maxIndex;
};

struct maxNValuesWithMatrixIndices {
	std::vector<double> maxNValues;
	std::vector<int> maxIndicesx;
	std::vector<int> maxIndicesy;
};

void findMaxOverRange(struct maxWithIndex*, double*, int, int, bool);
std::vector<idx_pair> getLargest(std::vector<double> const&, int);
void findMaxNValuesOverMatrixFast(struct maxNValuesWithMatrixIndices*, double**, int, int, int, int, int);
void findMaxNValuesOverMatrix(struct maxNValuesWithMatrixIndices*, double**, int, int, int, int, int);
std::string getMatrixString(double**, int, int);
std::string getMatrixString(int**, int, int);

void createCSV(std::vector<std::vector<double>>, std::string, std::string);
