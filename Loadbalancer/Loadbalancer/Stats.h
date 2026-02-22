#pragma once
#include "pch.h"

class Stats
{
public:
	Stats(std::queue<Request>* rQ, int* nS) { requestQueue = rQ; numServers = nS; };
	void printStats();

private:
	std::queue<Request>* requestQueue;
	int* numServers;
};