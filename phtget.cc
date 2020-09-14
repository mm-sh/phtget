#include <unistd.h>

#include <iostream>
#include <string>
#include <sstream>

#include "Downloader.h"
#include "Parser.h"

namespace {
    void printUsage(const char *progName) {
        std::cerr << "USAGE: " << progName;
        std::cerr << " [-v] [-p] url" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    bool verbose = false, showProgress = false;
    int opt;
    std::string header;
    std::ostringstream oss;

    Downloader loader;
    Parser parser;
    RequestInfo reqInfo;

    if (argc < 2 || argc > 4) {
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "vp")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'p':
                showProgress = true;
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        std::cerr << "ERROR: url missing!" << std::endl;
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    reqInfo.url = argv[optind];
    bool parseRes = parser.parseUrl(reqInfo);

    if (verbose) {
        std::cout << "INFO: URL: " << reqInfo.url << std::endl;
        std::cout << "INFO: PROTOCOL: " << reqInfo.protocol << std::endl;
        std::cout << "INFO: HOST: " << reqInfo.host << std::endl;
        std::cout << "INFO: PORT: " << reqInfo.port << std::endl;
        std::cout << "INFO: FILENAME: " << reqInfo.fileName << std::endl;
    }

    if (!parseRes) {
        std::cerr << "ERROR: ONLY HTTP PROTOCOL SUPPORTED!" << std::endl;
        exit(EXIT_FAILURE);
    }

    oss << "GET " << reqInfo.url.c_str() << " HTTP/1.1\r\n";
    oss << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
    oss << "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n";
    oss << "Host: " << reqInfo.host.c_str() << "\r\n";
    oss << "Connection: keep-alive\r\n";
    oss << "\r\n";
    header = oss.str();

    std::string response = loader.establishConnection(reqInfo.host, reqInfo.port, header, verbose);
    if (response.empty()) {
        std::cerr << "ERROR: RESPONSE RECIEVING FAILED!" << std::endl;
        exit(EXIT_FAILURE);
    }
    ResponseHeader resp = parser.parseHeader(response);
    if (verbose) {
        std::cout << "INFO: RESPONSE STATUS CODE: " << resp.statusCode << std::endl;
        std::cout << "INFO: RESPONSE CONTENT TYPE: " << resp.contentType << std::endl;
        std::cout << "INFO: RESPONSE CONTENT LENGTH: " << resp.contentLength << std::endl;
    }

    if (resp.statusCode >= 400 && resp.statusCode <= 505) {
        std::cerr << "ERROR: HTTP " << resp.statusCode << " ERROR!" << std::endl;
        exit(EXIT_FAILURE);
    }

    bool downloadRes = loader.download(reqInfo.fileName, resp.contentLength,
            showProgress);

    if (verbose && downloadRes) {
        std::cout << "INFO: FILE " << reqInfo.fileName;
        std::cout << " DOWNLOADED SUCCESSFULLY!" << std::endl;
        exit(EXIT_FAILURE);
    } else if (verbose && !downloadRes) {
        std::cout << "INFO: FILE " << reqInfo.fileName;
        std::cout << " DOWNLOADING FAILED!" << std::endl;
    }

    exit(EXIT_SUCCESS);
}
