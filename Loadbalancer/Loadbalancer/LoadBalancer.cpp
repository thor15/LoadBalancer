#include "pch.h"

void LoadBalancer::startInitial(int* numServers)
{
	for (int i = 0; i < *numServers; i++)
	{
		WebServer* server = new WebServer(serverQueue, ctxServer);
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
			break;
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
	}
	
	std::unique_lock<std::shared_mutex> lock(addRemove->rw);
	cv.wait(lock, [&] { return scalerDone->load(std::memory_order_acquire); });
	
	for (int i = 0; i < *serverCount; i++)
	{
		ctxServer->sem.acquire();
		WebServer* server;
		{
			std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
			server = serverQueue->front();
			serverQueue->pop();
		}

		delete server;
	}
}