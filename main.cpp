#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <exception>
#include <algorithm>

#define TIME_OUT 120

template <class Problem,class Solution> class Solver;
template <class Problem,class Solution> class CacheManager;
class MyTestClientHandler;
class ClientHandler;
class Server;

template <class Problem,class Solution> class Solver {
public:
    virtual Solution solve(Problem) = 0;
};
class StringReverser: public Solver<std::string,std::string> {
public:
    std::string solve(std::string problem) override {

        std::string solution(problem.rbegin(), problem.rend());
        return solution;
    }
};
template <class Problem,class Solution> class CacheManager {
public:
    virtual Solution findSolution(Problem p) = 0;
    virtual void saveSolution(Solution s) = 0;

};
template <class Problem,class Solution> class FileCacheManager: public CacheManager<Problem,Solution> {

};
class ClientHandler {
public:
    virtual void handleClient(int client_socket) = 0;
};
class MyTestClientHandler: public ClientHandler {
private:
    CacheManager<int, int>* cm;
    Solver<int, int>* solver;
public:
    MyTestClientHandler(CacheManager<int, int> *cm, Solver<int, int> *solver) : cm(cm), solver(solver) {}

    void handleClient(int client_socket) override {

        char buffer[1024] = {0};
        read(client_socket, buffer, 1024);
        std::string s(buffer);
    }
};
class Server {
public:
    virtual int open(int port, ClientHandler* c) = 0;
    virtual void stop() = 0;
};
class MySerialServer: public Server {
private:
    bool m_stop = false;
    std::thread t1;
public:
    int open(int port, ClientHandler* c) override {
        //_t1 = std::thread(&MySerialServer::open, this, port);

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == sockfd) {
            //error
            // return -1;
        }
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        int result = bind(sockfd, (struct sockaddr *) &address, sizeof(address));
        if (result == -1) {
            //error
            return -2;
        }
        if (listen(sockfd, 5) == -1) {
            //error
            return -3;
        }
        while (!m_stop) {

            struct timeval tv = {TIME_OUT,0};
            //tv.tv_sec = TIME_OUT;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        int client_socket = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &address);
        if (client_socket == -1) {
            //error
            return -4;
        }

        // closing the listening socket
        close(sockfd);
        c->handleClient(client_socket);
        }
    }
    void stop() override {
        this->m_stop = true;
    }
};


int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    std::thread t;

    ClientHandler* c = new MyTestClientHandler();
    Server* my_server = new MySerialServer();
    my_server->open(std::stoi(argv[1]),c);
    //t = thread(&MySerialServer::open, std::stoi(argv[1]), &c);

    return 0;
}
