#pragma once
#include "pch.h"

class Stats
{
public:
	Stats(std::queue<Request>* rQ, int* nS, FILE* logs, std::shared_ptr<std::atomic<bool>> rD, std::queue<WebServer*>* sQ) 
	{
		requestQueue = rQ; numServers = nS; logFile = logs; requestDone = std::move(rD); serverQueue = sQ; averageRPS = 0; numPulls = 0;
	};
	void printStats();

private:
	std::queue<Request>* requestQueue;
	std::queue<WebServer*>* serverQueue;
	int* numServers;
	FILE* logFile;
	std::shared_ptr<std::atomic<bool>> requestDone;
	float averageRPS;
	int numPulls;
};