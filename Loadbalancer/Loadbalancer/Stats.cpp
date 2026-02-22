#include "pch.h"

void Stats::printStats()
{
	while (true)
	{
		printf("Number of Servers: %d, Current Number of Requests: %lld", *numServers, requestQueue->size());

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}
}