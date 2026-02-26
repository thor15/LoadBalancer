#include "pch.h"

void Scaler::monitorServers()
{
	int cyclesSince = 170;
	while (true)
	{
		if (cyclesSince < cyclesBetween)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40 * MILI_PER_CYCLE));
			cyclesSince += 40;
			continue;
		}
		int numRequsts = requestQueue->size();

		if (numRequsts > *numServers * 80)
		{
			printf("Adding Server\n");
			addServer();
			cyclesSince = 0;
		}
		else if (numRequsts < *numServers * 50 && *numServers > 1)
		{
			printf("Removing Server\n");
			removeServer();
			cyclesSince = 0;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(40  * MILI_PER_CYCLE));

		if (requestDone->load() && *numServers == 1)
		{
			break;
		}
	}
	{
		std::lock_guard<std::shared_mutex> lock(addRemove->rw);
		scalerDone->store(true);
	}
	
}

void Scaler::addServer()
{
	WebServer* server = new WebServer(serverQueue, ctxServer);
	server->start();
	{
		std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
		serverQueue->push(server);
	}
	ctxServer->sem.release();
	(*numServers)++;
}

void Scaler::removeServer()
{
	WebServer* server;
	addRemove->sem.acquire();
	ctxServer->sem.acquire();
	{
		std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
		server = serverQueue->front();
		serverQueue->pop();
	}
	addRemove->sem.release();
	delete server;
	(*numServers)--;
}