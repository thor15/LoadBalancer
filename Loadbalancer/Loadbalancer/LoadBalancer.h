#pragma once
#ifndef LOADBALANCER_H
#define LOADBALANCER_H
#include "pch.h"

class LoadBalancer
{
public:
	LoadBalancer(std::queue<Request>* rQ, std::queue<WebServer*>* sQ, std::shared_ptr<SyncContext> sctx, std::shared_ptr<SyncContext> sctxS, std::shared_ptr<SyncContext> aR,
		std::shared_ptr<std::atomic<bool>> sD) : serverCount(0) {
		requestQueue = rQ; serverQueue = sQ; ctxRequest = std::move(sctx);	ctxServer = std::move(sctxS); addRemove = std::move(aR); scalerDone = std::move(sD);
	};
	void startInitial(int* numServers);
	void processRequests();

private:
	std::queue<Request>* requestQueue;
	std::queue<WebServer*>* serverQueue;
	int* serverCount;
	std::shared_ptr<SyncContext> ctxRequest;
	std::shared_ptr<SyncContext> ctxServer;
	std::shared_ptr<SyncContext> addRemove;
	std::shared_ptr<std::atomic<bool>> scalerDone;
	std::condition_variable_any cv;
};


#endif // !LOADBALANCER_H
