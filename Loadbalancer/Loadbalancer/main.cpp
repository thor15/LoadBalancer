#include "pch.h"

int main(int args, char** argv)
{
	if (args < 4)
	{
		printf("Too few arguments");
		return 1;
	}
	int numServers = atoi(argv[1]);
	int duration = atoi(argv[2]);
	int mode = atoi(argv[3]);
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

	
	GenerateRequest generator = GenerateRequest(requestMode, &requestQueue, ctxRequest, requestDone);
	LoadBalancer balancer = LoadBalancer(&requestQueue, &serverQueue, ctxRequest, ctxServer, addRemove, scalerDone);
	Scaler scaler = Scaler(&requestQueue, &serverQueue, &numServers, ctxServer, addRemove, requestDone, scalerDone);
	Stats stats = Stats(&requestQueue, &numServers);
	

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
	printf("Merging with GenerateRequests\n");
	scalerthread.join();
	printf("Merging with Scaler\n");
	loadbalancer.join();
	printf("Merging with LoadBalancer\n");
	statthread.detach();
	
	
	return 0;
}