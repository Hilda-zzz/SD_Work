#include "ThreadPool.hpp"
#include <iostream>

ThreadPool::ThreadPool(size_t threadsCount)
{
	std::cout << "Pool Construct:" << threadsCount << std::endl;
}

ThreadPool::~ThreadPool()
{
	std::cout << "Pool Deconstruct" << std::endl;
}
