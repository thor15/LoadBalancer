#pragma once

#include "pch.h"

class Scaler
{
public:
	Scaler(std::queue<Request>* rQ, std::queue<WebServer*>* sQ, int* nS, std::shared_ptr<SyncContext> sctxS, std::shared_ptr<SyncContext> aR, std::shared_ptr<std::atomic<bool>> rD, 
		std::shared_ptr<std::atomic<bool>> sD) {
		requestQueue = rQ; serverQueue = sQ; numServers = nS; ctxServer = std::move(sctxS); addRemove = std::move(aR); requestDone = std::move(rD); scalerDone = std::move(sD);
	};
	void monitorServers();
	

private:
	void addServer();
	void removeServer();
	std::queue<Request>* requestQueue;
	std::queue<WebServer*>* serverQueue;
	int* numServers;
	int cyclesBetween = 160;
	std::shared_ptr<SyncContext> ctxServer;
	std::shared_ptr<SyncContext> addRemove;
	std::shared_ptr<std::atomic<bool>> requestDone;
	std::shared_ptr<std::atomic<bool>> scalerDone;
};