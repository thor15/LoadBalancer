#include "pch.h"

void WebServer::processRequest(Request request)
{
	{
		std::lock_guard<std::mutex> lk(rq_m);
		requestQueue.push(request);
	}
	
	sem.release();
}

WebServer::~WebServer()
{
	stop();
	join();
}

void WebServer::start()
{
	running.store(true, std::memory_order_release);
	worker_ = std::thread(&WebServer::run, this);
}

void WebServer::stop()
{
	running.store(false, std::memory_order_release);
	sem.release();
}

void WebServer::join()
{
	if (worker_.joinable()) worker_.join();
}

void WebServer::run()
{
	while (true)
	{
		sem.acquire();

		if (!running.load(std::memory_order_acquire))
		{
			(*idleServer)++;
			break;
		}

		Request request;
		{
			std::lock_guard<std::mutex> lk(rq_m);
			request = requestQueue.front();
		}
		requestQueue.pop();
		std::this_thread::sleep_for(std::chrono::milliseconds(request.duration * MILI_PER_CYCLE));
		//printf("Finished a Request!\n");
		{
			std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
			serverQueue->push(this);
		}
		ctxServer->sem.release();
		if (!running.load(std::memory_order_acquire))
		{
			(*activeServer)++;
			break;
		}
	}

	//printf("done!\n");
}