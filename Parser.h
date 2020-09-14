#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string>

struct RequestInfo {
    std::string url;
    std::string protocol;
    std::string host;
    std::string fileName;
    int port;
};

struct ResponseHeader {
    int statusCode;
    std::string contentType;
    long contentLength;
};

class Parser {
public:
    Parser() {};
    ~Parser() {};
    bool parseUrl(RequestInfo &requestInfo);
    ResponseHeader parseHeader(std::string response);
};

#endif // URL_PARSER_H
