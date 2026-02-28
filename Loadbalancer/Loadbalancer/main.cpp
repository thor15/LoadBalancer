#include "pch.h"

int main(int args, char** argv)
{
	if (args < 4)
	{
		printf("Too few arguments\n");
		return 1;
	}
	int numServers = atoi(argv[1]);
	int duration = atoi(argv[2]);
	int mode = atoi(argv[3]);
	FILE* logFile = nullptr;

	#ifdef _MSC_VER
		errno_t err = fopen_s(&logFile, "log.txt", "w");
		if (err != 0 || !logFile)
	#else
		logFile = fopen("log.txt", "w");
		if (!logFile)
	#endif
	{
		printf("Failed to open log\n");
		return 1;
	}


	printf("Request times are inbetween 50 and 110 cycles\n");
	fprintf(logFile, "Request times are inbetween 50 and 110 cycles\n");
	printf("There are %d initial requests\n", numServers * 10);
	fprintf(logFile, "There are %d initial requests\n", numServers * 10);
	

	RequestMode requestMode = RequestMode::SLOW;
	if (mode == 0)
	{
		requestMode = RequestMode::BURST;
	}
	else if (mode == 1)
	{
		requestMode = RequestMode::NORMAL;
	}

	int regectedRequest = 0;
	int acceptedReqest = 0;
	int activeServer = 0;
	int idleServer = 0;
	std::queue<Request>    requestQueue;
	std::queue<WebServer*> serverQueue;

	auto ctxRequest = std::make_shared<SyncContext>();
	auto ctxServer = std::make_shared<SyncContext>();
	auto addRemove = std::make_shared<SyncContext>();

	auto requestDone = std::make_shared<std::atomic<bool>>();
	auto scalerDone = std::make_shared < std::atomic<bool>>();

	
	GenerateRequest generator = GenerateRequest(requestMode, &requestQueue, ctxRequest, requestDone, logFile);
	LoadBalancer balancer = LoadBalancer(&requestQueue, &serverQueue, ctxRequest, ctxServer, addRemove, scalerDone, logFile, requestDone, &regectedRequest, &acceptedReqest, &activeServer, &idleServer);
	Scaler scaler = Scaler(&requestQueue, &serverQueue, &numServers, ctxServer, addRemove, requestDone, scalerDone, logFile, &activeServer, &idleServer);
	Stats stats = Stats(&requestQueue, &numServers, logFile, requestDone, &serverQueue);
	

	balancer.startInitial(&numServers);
	generator.generateInitialRequests(numServers);

	std::thread producer([&] {
		generator.continuouslyCreateRequests(duration);
	});

	std::thread loadbalancer([&] {
		balancer.processRequests();
	});

	std::thread statthread([&] {
		stats.printStats();
	});

	std::thread scalerthread([&] {
		scaler.monitorServers();
	});
	
	producer.join();
	scalerthread.join();
	loadbalancer.join();
	statthread.join();

	int iter = numServers;
	for (int i = 0; i < iter; i++)
	{
		ctxServer->sem.acquire();
		WebServer* server;
		{
			std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
			server = serverQueue.front();
			serverQueue.pop();
		}
		delete server;
	}
	printf(GREEN "Done cleaning up!\n" RESET);
	fprintf(logFile, "Done cleaning up!\n");
	
	printf("There were %d idles servers and %d active servers\n", idleServer, activeServer);
	fprintf(logFile, "There were %d idles servers and %d active servers\n", idleServer, activeServer);
	printf("There were %d requests left incomplete\n", static_cast<int>(requestQueue.size()));
	fprintf(logFile, "There were %d requests left incomplete\n", static_cast<int>(requestQueue.size()));
	printf("%d requests were rejected and %d requests were processed\n", regectedRequest, acceptedReqest);
	fprintf(logFile, "%d requests were rejected and %d requests were processed\n", regectedRequest, acceptedReqest);

	fclose(logFile);
	
	return 0;
}