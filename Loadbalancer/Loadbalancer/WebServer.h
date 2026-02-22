#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H


class WebServer
{
public:
	WebServer(std::queue<WebServer*>* sQ, std::shared_ptr<SyncContext> sctx) { serverQueue = sQ; ctxServer = std::move(sctx); };
	void processRequest(Request request);

private:
	std::queue<WebServer*>* serverQueue;
	std::shared_ptr<SyncContext> ctxServer;
};

#endif // !WEBSERVER_H
