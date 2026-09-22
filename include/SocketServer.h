#pragma once
#include <string>
#include <atomic>
#include <thread>

class SocketServer {
public:
    SocketServer(int port);
    ~SocketServer();

    void start();
    void stop();

private:
    int port_;
    int listenFd_;
    std::atomic<bool> running_;

    void handleClient(int clientFd, std::string clientIp);
};