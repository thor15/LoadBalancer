#include "pch.h"

std::string GenerateRequest::generateIP()
{
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> octet(0, 255);

    return std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng));
}

void GenerateRequest::continuouslyCreateRequests(int totalTime)
{
    int cTime = 0;
    while (cTime < totalTime)
    {
        std::string inIP = generateIP();
        std::string outIP = generateIP();

        Request newRequest = Request();
        newRequest.requestIP = inIP;
        newRequest.destinationIP = outIP;
        newRequest.duration = rand() % (26) + 10;
        int type = rand() % 2;
        if (type == 0)
        {
            newRequest.jobType = S;
        }
        else
        {
            newRequest.jobType = P;
        }

        {
            std::unique_lock<std::shared_mutex> lock(ctx->rw);
            requestQueue->push(newRequest);
        }
        ctx->sem.release();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cTime += 1;
    }

    Request newRequest = Request();
    newRequest.requestIP = "-1";
    newRequest.destinationIP = "-1";
    {
        std::unique_lock<std::shared_mutex> lock(ctx->rw);
        requestQueue->push(newRequest);
    }

    requestDone->store(true);
}

void GenerateRequest::generateInitialRequests(int numServers)
{
    for (int i = 0; i < numServers * 100; i++)
    {
        std::string inIP = generateIP();
        std::string outIP = generateIP();

        Request newRequest = Request();
        newRequest.requestIP = inIP;
        newRequest.destinationIP = outIP;
        newRequest.duration = rand() % (26) + 10;
        int type = rand() % 2;
        if(type == 0)
        {
            newRequest.jobType = S;
        }
        else
        {
            newRequest.jobType = P;
        }

        requestQueue->push(newRequest);
        ctx->sem.release();
    }
}