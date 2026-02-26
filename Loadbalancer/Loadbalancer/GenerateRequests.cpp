#include "pch.h"
int GenerateRequest::next_interarrival_ms(std::mt19937& rng)
{
    switch (requestMode)
    {
        case RequestMode::BURST:
        {
            if (remainingInBurst <= 0)
            {
                std::uniform_int_distribution<int> burst_size_dist(300, 800);
                remainingInBurst = burst_size_dist(rng);

                std::uniform_int_distribution<int> pause_dist(400, 800);
                return pause_dist(rng);
            }
            remainingInBurst--;
            std::uniform_int_distribution<int> short_dist(1, 3);
            return short_dist(rng);
        }

        case RequestMode::SLOW:
        {
            std::uniform_int_distribution<int> slow_dist(30, 150);
            return slow_dist(rng);
        }

        case RequestMode::NORMAL:
        default:
        {
            constexpr int mean = 25;
            std::normal_distribution<double> d(mean, 12.0);
            int ms = static_cast<int>(d(rng));
            return std::clamp(ms, 6, 15);
        }
    }
}


std::string GenerateRequest::generateIP()
{
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> octet(0, 255);

    return std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng)) + "." +
        std::to_string(octet(rng));
}

std::string GenerateRequest::generateRequestIP(std::mt19937& rng)
{
    if (!destinationDoS)
    {
        if (requestDoS)
        {
            remainingInDoS--;
            requestDoS = remainingInBurst == 0;
            return DoSIP;
        }

        std::uniform_int_distribution<int> oneInHundred(1, 100);
        if (oneInHundred(rng) == 1)
        {
            requestDoS = true;
            remainingInDoS = 100;
            DoSIP = generateIP();
            return DoSIP;
        }
    }

    return generateIP();
}

std::string GenerateRequest::generateDestinationIP(std::mt19937& rng)
{
    if (!requestDoS)
    {
        if (destinationDoS)
        {
            remainingInDoS--;
            destinationDoS = remainingInBurst == 0;
            return DoSIP;
        }

        std::uniform_int_distribution<int> oneInHundred(1, 100);
        if (oneInHundred(rng) == 1)
        {
            destinationDoS = true;
            remainingInDoS = 100;
            DoSIP = generateIP();
            return DoSIP;
        }   
    }

    return generateIP();
}

void GenerateRequest::continuouslyCreateRequests(int totalTime)
{
    std::mt19937 rng(std::random_device{}());
    int cTime = 0;
    while (cTime < totalTime)
    {
        std::string inIP = generateIP();
        std::string outIP = generateIP();

        Request newRequest = Request();
        newRequest.requestIP = inIP;
        newRequest.destinationIP = outIP;
        newRequest.duration = rand() % (60) + 50;
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
        int sleep_cycle = next_interarrival_ms(rng);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_cycle *MILI_PER_CYCLE));
        cTime += 1;
    }

    Request newRequest = Request();
    newRequest.requestIP = "-1";
    newRequest.destinationIP = "-1";
    {
        std::unique_lock<std::shared_mutex> lock(ctx->rw);
        requestQueue->push(newRequest);
    }
    ctx->sem.release();
    requestDone->store(true);
    printf("Requests Done!\n");
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