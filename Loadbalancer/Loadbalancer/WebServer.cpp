#include "pch.h"

void WebServer::processRequest(Request request)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(request.duration * 200));

	{
		std::unique_lock<std::shared_mutex> lock(ctxServer->rw);
		serverQueue->push(this);
	}
	ctxServer->sem.release();
}

void WebServer::start()
{

}