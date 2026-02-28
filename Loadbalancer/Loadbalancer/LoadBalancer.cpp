#include "pch.h"

void LoadBalancer::startInitial(int* numServers)
{
	for (int i = 0; i < *numServers; i++)
	{
		WebServer* server = new WebServer(serverQueue, ctxServer, activeServer, idleServer);
		server->start();
		serverQueue->push(server);
	}
	serverCount = numServers;
	ctxServer->sem.release(*numServers);
	addRemove->sem.release();
}

void LoadBalancer::processRequests()
{
	std::mt19937 rng(std::random_device{}());
	while (true)
	{
		if (requestDone->load())
		{
			break;
		}

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
			fprintf(logFile, "All requests have been sent to processing\n");
			break;
		}


		if (allowRequest(request))
		{
			std::uniform_int_distribution<int> oneInHundred(1, 300);
			if (oneInHundred(rng) == 1)
			{
				printf("Recived request with requestid: %s, destinationid: %s, and duration: %d\n", request.requestIP.c_str(), request.destinationIP.c_str(), request.duration);
				fprintf(logFile, "Recived request with requestid: %s, destinationid: %s, and duration: %d\n", request.requestIP.c_str(), request.destinationIP.c_str(), request.duration);
			}

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
			(*acceptedRequest)++;
		}
		else if (firstFind)
		{
			(*regectedRequest)++;
		}
		else
		{
			(*regectedRequest)++;
		}
	}
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
		if (!e.inUse)
		{
			continue;
		}
			
		e.score -= decay;
		if (e.score < 0)
		{
			e.score = 0;
		}
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
		printf(RED "Stopping a DoS Attack with IP %s" RESET "\n", list[0].ip.c_str());
		fprintf(logFile, "Stopping a DoS Attack with IP %s\n", list[0].ip.c_str());
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