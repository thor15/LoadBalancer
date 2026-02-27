#pragma once
#include "pch.h"

class Stats
{
public:
	Stats(std::queue<Request>* rQ, int* nS, FILE* logs) { requestQueue = rQ; numServers = nS; logFile = logs; };
	void printStats();

private:
	std::queue<Request>* requestQueue;
	int* numServers;
	FILE* logFile;
};