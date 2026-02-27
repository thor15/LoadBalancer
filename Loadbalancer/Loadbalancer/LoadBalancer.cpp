#include "pch.h"

void LoadBalancer::startInitial(int* numServers)
{
	for (int i = 0; i < *numServers; i++)
	{
		WebServer* server = new WebServer(serverQueue, ctxServer);
		server->start();
		serverQueue->push(server);
	}
	serverCount = numServers;
	ctxServer->sem.release(*numServers);
	addRemove->sem.release();
}

void LoadBalancer::processRequests()
{
	int timeSinceServer = 10;
	bool drop = false;
	bool add = false;
	while (true)
	{
		ctxRequest->sem.acquire();

		Request request;
		size_t qsize;
		{
			std::unique_lock<std::shared_mutex> lock(ctxRequest->rw);
			request = requestQueue->front();
			requestQueue->pop();
			qsize = requestQueue->size();
		}

		if (request.requestIP == "-1" && request.destinationIP == "-1")
		{
			printf(GREEN "All requests have been sent to processing\n" RESET);
			break;
		}


		if (allowRequest(request))
		{
			addRemove->sem.acquire();
			addRemove->sem.release();
			ctxServer->sem.acquire();
			WebServer* server;
			{
				std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
				server = serverQueue->front();
				serverQueue->pop();
			}
			server->processRequest(request);
		}
		else if (firstFind)
		{
			printf(RED "Stopping a DoS Attack" RESET "\n");
		}
	}
	
	std::unique_lock<std::shared_mutex> lock(addRemove->rw);
	cv.wait(lock, [&] { return scalerDone->load(std::memory_order_acquire); });
	int iter = *serverCount;
	for (int i = 0; i < iter; i++)
	{
		ctxServer->sem.acquire();
		WebServer* server;
		{
			std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
			server = serverQueue->front();
			serverQueue->pop();
		}
		(*serverCount)--;
		delete server;
	}
	printf(GREEN "Done cleaning up!\n" RESET);
}

int LoadBalancer::find(const std::array<HotEntry, K>& list, const std::string& ip)
{
	for (int i = 0; i < K; ++i) {
		if (list[i].inUse && list[i].ip == ip)
		{
			return i;
		}
	}
	return -1;
}

void LoadBalancer::moveToFront(std::array<HotEntry, K>& list, int i)
{
	if (i <= 0)
	{
		return;
	}
	HotEntry tmp = std::move(list[i]);
	for (int j = i; j > 0; --j)
	{
		list[j] = std::move(list[j - 1]);
	}
	list[0] = std::move(tmp);
}

void LoadBalancer::insertFront(std::array<HotEntry, K>& list, const std::string& ip, int initialScore)
{
	for (int j = K - 1; j > 0; --j)
	{
		list[j] = std::move(list[j - 1]);
	}
			
	list[0] = HotEntry{};
	list[0].ip = ip;
	list[0].score = initialScore;
	list[0].inUse = true;
}

bool LoadBalancer::allowByHotList(const std::string& ip, std::array<HotEntry, K>& list)
{
	firstFind = false;
	for (auto& e : list) 
	{
		if (!e.inUse) continue;
		e.score -= decay;
		if (e.score < 0) e.score = 0;
	}

	int idx = find(list, ip);
	if (idx != -1 && reqSeq <= list[idx].blockedUntilSeq) 
	{
		moveToFront(list, idx);
		return false;
	}

	if (idx == -1) 
	{
		insertFront(list, ip, bump);
		return true;
	}

	list[idx].score += bump;
	moveToFront(list, idx);

	if (list[0].score >= blockThreshhold) 
	{
		list[0].blockedUntilSeq = reqSeq + blockLength;
		list[0].score = 0;
		firstFind = true;
		return false;
	}

	return true;
}

bool LoadBalancer::allowRequest(Request request)
{
	++reqSeq;

	
	if (!allowByHotList(request.requestIP, srcHot) || !allowByHotList(request.destinationIP, dstHot))
	{
		return false;
	}

	return true;
}