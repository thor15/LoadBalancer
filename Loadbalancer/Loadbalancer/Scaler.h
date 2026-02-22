#pragma once

#include "pch.h"

class Scaler
{
public:
	Scaler(std::queue<Request>* rQ, std::queue<WebServer*>* sQ, int* nS, std::shared_ptr<SyncContext> sctxS, std::shared_ptr<SyncContext> aR) { 
		requestQueue = rQ; serverQueue = sQ; numServers = nS; ctxServer = std::move(sctxS); addRemove = std::move(aR);
	};
	void monitorServers();
	

private:
	void addServer();
	void removeServer();
	std::queue<Request>* requestQueue;
	std::queue<WebServer*>* serverQueue;
	int* numServers;
	std::shared_ptr<SyncContext> ctxServer;
	std::shared_ptr<SyncContext> addRemove;
};