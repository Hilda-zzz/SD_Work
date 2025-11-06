#pragma once
class RandomNumberGenerator;
class IntRange
{
public:
	int m_min;
	int m_max;
	static const  IntRange ZERO;
	static const  IntRange ONE;
	static const  IntRange ZERO_TO_ONE;
public:
	~IntRange() {}
	IntRange();
	explicit IntRange(int min, int max);

	bool IsOnRange(int inputInt);
	bool IsOverlappipngWith(IntRange const& inputIntRange);
	int GetRandomInRange(RandomNumberGenerator* rng) const;

	void operator=(IntRange const& copyFrom);
	bool operator==(IntRange const& copyFrom);
	bool operator!=(IntRange const& copyFrom);
};