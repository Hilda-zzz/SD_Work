#include "RandomNumberGenerator.hpp"
#include <stdlib.h>
#include <time.h>
int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	//srand((unsigned int)time(NULL));
	return rand() % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	int range = (maxInclusive - minInclusive) + 1;
	return minInclusive + RollRandomIntLessThan(range);
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	//srand((unsigned int)time(NULL));
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float zeroToOne= RollRandomFloatZeroToOne();
	//int range =maxInclusive - minInclusive;
	return minInclusive + (zeroToOne * (maxInclusive - minInclusive));
}