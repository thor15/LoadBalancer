#include "pch.h"

void Scaler::monitorServers()
{
	int cyclesSince = 170;
	while (true)
	{
		if (requestDone->load())
		{
			break;
		}

		if (cyclesSince < cyclesBetween)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(60 * MILI_PER_CYCLE));
			cyclesSince += 40;
			continue;
		}
		int numRequsts = requestQueue->size();

		if (numRequsts > *numServers * 80)
		{
			printf(GREEN "Adding Server\n" RESET);
			fprintf(logFile, "Adding Server\n");
			addServer();
			cyclesSince = 0;
		}
		else if (numRequsts < *numServers * 50 && *numServers > 1)
		{
			printf(RED "Removing Server\n" RESET);
			fprintf(logFile, "Removing Server\n");
			removeServer();
			cyclesSince = 0;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(40 * MILI_PER_CYCLE));
	}
	{
		std::lock_guard<std::shared_mutex> lock(addRemove->rw);
		scalerDone->store(true);
	}
	
}

void Scaler::addServer()
{
	WebServer* server = new WebServer(serverQueue, ctxServer, activeServer, idleServer);
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