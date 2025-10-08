#pragma once
class Job
{
public:
	virtual ~Job() {}

	virtual void Execute() = 0;
};