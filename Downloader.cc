#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "Downloader.h"

namespace {
    void printBar(long recieved, long total) {
        int barWidth = 60;
        float progress = (float)recieved / total;

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; i++) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
    }
}

bool Downloader::download(std::string fileName, long contentLength,
        bool showProgress) {

    long recievedBytes = 0;
    int memSize = 8192;
    int bufLen = memSize;
    int len;

    int fd = open(fileName.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        std::cerr << "ERROR: FILE OPENING FAILURE!" << std::endl;
        return false;
    }

    char *buf = new char[memSize];

    while (recievedBytes < contentLength) {
        len = read(clientSocket, buf, bufLen);
        write(fd, buf, len);
        recievedBytes += len;
        if (showProgress) {
            printBar(recievedBytes, contentLength);
        }
        if (recievedBytes == contentLength) {
            break;
        }
    }
    std::cout << std::endl;
    delete[] buf;
    return true;
}

void Downloader::disposeConnection() {
    freeaddrinfo(res);
    close(clientSocket);
}

std::string Downloader::establishConnection(std::string host, int port,
        std::string header, bool verbose) {
    char ipAddr[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    std::string portStr = std::to_string(port);
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0) {
        std::cerr << "ERROR: FAILED TO GET ADDRESS!" << std::endl;
        return {};
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        clientSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSocket == -1) {
            std::cerr << "ERROR: FAILED TO CREATE SOCKET! RETRYING..." << std::endl;
            continue;
        }
        if (connect(clientSocket, rp->ai_addr, rp->ai_addrlen) == -1) {
            std::cerr << "ERROR: FAILED TO CONNECT! RETRYING..." << std::endl;
            continue;
        }
        break;
    }

    if (rp == nullptr) {
        std::cerr << "ERROR: SOCKET CREATION/CONNECTION FAILED!" << std::endl;
        return {};
    }

    if (verbose) {
        for (rp = res; rp != NULL; rp = rp->ai_next) {
            void *addr;
            if (rp->ai_family == AF_INET) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
                addr = &(ipv4->sin_addr);
            } else {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
                addr = &(ipv6->sin6_addr);
            }
            inet_ntop(rp->ai_family, addr, ipAddr, sizeof(ipAddr));
        }
        std::cout << "INFO: IP ADRESS: " << ipAddr << std::endl;
    }

    if (send(clientSocket, header.c_str(), header.size(), 0) == -1) {
        std::cerr << "ERROR: SENDING FAILED!" << std::endl;
        return {};
    }

    int readed = 0, respLen = 0;
    int memSize = 1024;
    int CRLF = 0; // 4 consecutive CRLF means end of header
    std::vector<char> buf(memSize, 0);

    while ((readed = read(clientSocket, &buf[respLen], 1)) != 0) {
        if (readed < 0) {
            std::cerr << "ERROR: READING FAILURE!" << std::endl;
            return {};
        }
        if (respLen + readed >= memSize) {
            memSize *= 2;
            buf.resize(memSize);
        }

        respLen += readed;
        buf[respLen] = '\0';

        if (buf[respLen - 1] == '\r' || buf[respLen - 1] == '\n') {
            CRLF++;
        } else {
            CRLF = 0;
        }

        if (CRLF == 4) {
            break;
        }
    }

    std::string response(buf.data());
    return response;
}
