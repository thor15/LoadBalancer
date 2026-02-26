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
			printf("All requests have been sent to processing\n");
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
	int iter = *serverCount;
	for (int i = 0; i < iter; i++)
	{
		printf("Deleting a server on iteration: %d\n", i);
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
	printf("Done cleaning up!\n");
}