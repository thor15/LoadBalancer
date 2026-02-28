#include "pch.h"

void Stats::printStats()
{
	while (true)
	{
		if (requestDone->load())
		{
			float finalAverage = averageRPS / numPulls;
			printf("On average there were %0.2f requests per server\n", finalAverage);
			fprintf(logFile, "On average there were %0.2f requests per server\n", finalAverage);
			break;
		}
		int numServer = *numServers;
		int numRequests = static_cast<int>(requestQueue->size());
		float requestPerServer = static_cast<float>(numRequests) / numServer;
		printf("Number of Servers: %d, Current Number of Requests: %d, With %0.2f requests per server\n", numServer, numRequests, requestPerServer);
		fprintf(logFile, "Number of Servers: %d, Current Number of Requests: %d, With %0.2f requests per server\n", numServer, numRequests, requestPerServer);
		averageRPS += requestPerServer;
		numPulls++;
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	}
}