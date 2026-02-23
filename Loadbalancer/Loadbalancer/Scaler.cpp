#include "pch.h"

void Scaler::monitorServers()
{
	while (true)
	{
		
		int numRequsts = requestQueue->size();
		printf("Making check, num requests: %d, num servers: %d\n", numRequsts, *numServers);

		if (requestDone->load())
		{
			break;
		}

		if (numRequsts > *numServers * 80)
		{
			addServer();
		}
		else if (numRequsts < *numServers * 50)
		{
			removeServer();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	}
	{
		std::lock_guard<std::shared_mutex> lock(addRemove->rw);
		scalerDone->store(true);
	}
	
}

void Scaler::addServer()
{
	WebServer* server = new WebServer(serverQueue, ctxServer);

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