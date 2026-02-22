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
	GenerateRequest(RequestMode rM, std::queue<Request>* rQ, std::shared_ptr<SyncContext> sctx) : requestMode(rM) { 
		requestQueue = rQ; ctx = std::move(sctx);
	};
	std::string generateIP();
	void continuouslyCreateRequests(int totalTime);
	void generateInitialRequests(int numServers);


private:
	RequestMode requestMode;
	std::queue<Request>* requestQueue;
	std::shared_ptr<SyncContext> ctx;
};




#endif // !GENERATEREQUESTS_H