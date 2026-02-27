#pragma once
#ifndef GENERATEREQUESTS_H
#include "pch.h"



enum RequestMode
{
	BURST = 1,
	SLOW = 2,
	NORMAL = 3
};

class GenerateRequest
{
public:
	GenerateRequest(RequestMode rM, std::queue<Request>* rQ, std::shared_ptr<SyncContext> sctx, std::shared_ptr<std::atomic<bool>> rD, FILE* logs) : requestMode(rM) {
		requestQueue = rQ; ctx = std::move(sctx); requestDone = std::move(rD); logFile = logs;
	};
	std::string generateIP();
	std::string generateRequestIP(std::mt19937& rng);
	std::string generateDestinationIP(std::mt19937& rng);
	void continuouslyCreateRequests(int totalTime);
	void generateInitialRequests(int numServers);


private:
	int next_interarrival_ms(std::mt19937& rng);
	RequestMode requestMode;
	std::queue<Request>* requestQueue;
	std::shared_ptr<SyncContext> ctx;
	std::shared_ptr<std::atomic<bool>> requestDone;
	int remainingInBurst = 0;
	std::string DoSIP;
	int remainingInDoS = 0;
	bool requestDoS = false;
	bool destinationDoS = false;
	FILE* logFile;
};




#endif // !GENERATEREQUESTS_H