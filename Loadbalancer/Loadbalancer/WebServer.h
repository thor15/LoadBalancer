#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H


class WebServer
{
public:
	WebServer(std::queue<WebServer*>* sQ, std::shared_ptr<SyncContext> sctx, int* acS, int* iS) : sem(0) { serverQueue = sQ; ctxServer = std::move(sctx); activeServer = acS; idleServer = iS; };
	~WebServer();
	void processRequest(Request request);
	void start();

private:
	void stop();
	void join();
	void run();

	std::queue<WebServer*>* serverQueue;
	std::shared_ptr<SyncContext> ctxServer;
	std::thread worker_;
	std::queue<Request> requestQueue;
	std::counting_semaphore<10> sem{ 0 };
	std::atomic<bool> running;
	std::mutex rq_m;
	int* activeServer;
	int* idleServer;
};

#endif // !WEBSERVER_H
