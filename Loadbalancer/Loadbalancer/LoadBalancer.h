#pragma once
#ifndef LOADBALANCER_H
#define LOADBALANCER_H
#include "pch.h"

class LoadBalancer
{
public:
	LoadBalancer(std::queue<Request>* rQ, std::queue<WebServer*>* sQ, std::shared_ptr<SyncContext> sctx, std::shared_ptr<SyncContext> sctxS, std::shared_ptr<SyncContext> aR,
		std::shared_ptr<std::atomic<bool>> sD, FILE* logs) : serverCount(0) {
		requestQueue = rQ; serverQueue = sQ; ctxRequest = std::move(sctx);	ctxServer = std::move(sctxS); addRemove = std::move(aR); scalerDone = std::move(sD); logFile = logs;
	};
	void startInitial(int* numServers);
	void processRequests();

	bool allowRequest(Request request);

private:
	struct HotEntry
	{
		std::string ip{};
		int score = 0;
		uint64_t blockedUntilSeq = 0;
		bool inUse = false;
	};

	static constexpr int K = 5;

	std::array<HotEntry, K> srcHot{};
	std::array<HotEntry, K> dstHot{};

	int find(const std::array<HotEntry, K>& list, const std::string& ip);
	void moveToFront(std::array<HotEntry, K>& list, int i);
	void insertFront(std::array<HotEntry, K>& list, const std::string& ip, int initialScore);
	bool allowByHotList(const std::string& ip, std::array<HotEntry, K>& list);

	std::queue<Request>* requestQueue;
	std::queue<WebServer*>* serverQueue;
	int* serverCount;
	std::shared_ptr<SyncContext> ctxRequest;
	std::shared_ptr<SyncContext> ctxServer;
	std::shared_ptr<SyncContext> addRemove;
	std::shared_ptr<std::atomic<bool>> scalerDone;
	std::condition_variable_any cv;
	int bump = 3;
	int decay = 1;
	int blockThreshhold = 20;
	int blockLength = 100;
	uint64_t reqSeq = 0;
	bool firstFind = false;
	FILE* logFile;
};


#endif // !LOADBALANCER_H
