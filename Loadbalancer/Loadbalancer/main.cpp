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
		errno_t err = fopen_s(&logFile, "../log.txt", "w");
		if (err != 0 || !logFile)
	#else
		logFile = fopen("../log.txt", "w");
		if (!logFile)
	#endif
	{
		printf("Failed to open log\n");
		return 1;
	}

	RequestMode requestMode = RequestMode::SLOW;
	if (mode == 0)
	{
		requestMode = RequestMode::BURST;
	}
	else if (mode == 1)
	{
		requestMode = RequestMode::NORMAL;
	}

	std::queue<Request>    requestQueue;
	std::queue<WebServer*> serverQueue;

	auto ctxRequest = std::make_shared<SyncContext>();
	auto ctxServer = std::make_shared<SyncContext>();
	auto addRemove = std::make_shared<SyncContext>();

	auto requestDone = std::make_shared<std::atomic<bool>>();
	auto scalerDone = std::make_shared < std::atomic<bool>>();

	
	GenerateRequest generator = GenerateRequest(requestMode, &requestQueue, ctxRequest, requestDone, logFile);
	LoadBalancer balancer = LoadBalancer(&requestQueue, &serverQueue, ctxRequest, ctxServer, addRemove, scalerDone, logFile);
	Scaler scaler = Scaler(&requestQueue, &serverQueue, &numServers, ctxServer, addRemove, requestDone, scalerDone, logFile);
	Stats stats = Stats(&requestQueue, &numServers, logFile);
	

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
	printf(YELLOW "Merging with GenerateRequests\n" RESET);
	fprintf(logFile, "Merging with GenerateRequests\n" );
	scalerthread.join();
	printf(YELLOW "Merging with Scaler\n" RESET);
	fprintf(logFile,  "Merging with Scaler\n" );
	loadbalancer.join();
	printf(YELLOW "Merging with LoadBalancer\n" RESET);
	fprintf(logFile,  "Merging with LoadBalancer\n" );
	statthread.detach();
	
	fclose(logFile);
	
	return 0;
}