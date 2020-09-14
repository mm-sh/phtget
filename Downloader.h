#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <netdb.h>

#include <string>

class Downloader {
    int clientSocket;
    struct addrinfo hints;
    struct addrinfo *res, *rp;

public:
    Downloader() {}
    ~Downloader() {}
    bool download(std::string fileName,
            long conentLength, bool showProgress);
    std::string establishConnection(std::string host, int port,
            std::string header, bool verbose);
    void disposeConnection();
};

#endif // DOWNLOADER_H
