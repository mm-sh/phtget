#include <string.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Parser.h"

bool Parser::parseUrl(RequestInfo &reqInfo) {

    size_t startPos = 0;
    std::string httpPrefix = "http://";

    if (reqInfo.url.compare(0, httpPrefix.size(), httpPrefix) == 0) {
            startPos = httpPrefix.size();
            reqInfo.protocol = httpPrefix;
    } else {
        return false;
    }

    // extract host name
    for (size_t i = startPos; reqInfo.url[i] != '/'
            && i != reqInfo.url.size(); i++) {
        reqInfo.host.push_back(reqInfo.url[i]);
    }

    // set port
    std::string portStr;
    std::size_t index = reqInfo.host.find(":");
    if (index != std::string::npos) {
        for (size_t i = index + 1; i < reqInfo.host.size(); i++) {
            portStr.push_back(reqInfo.host[i]);
        }
        reqInfo.port = stoi(portStr);
    } else {
        reqInfo.port = 80; // default
    }

    // extract filename
    std::string tempName;
    for (size_t i = startPos; i < reqInfo.url.size(); i++) {
        if (reqInfo.url[i] == '/' && i != reqInfo.url.size() - 1) {
            tempName.clear();
            continue;
        }
        tempName.push_back(reqInfo.url[i]);
    }
    reqInfo.fileName = tempName;
    return true;
}

ResponseHeader Parser::parseHeader(std::string response) {
    ResponseHeader resp;
    int responseSize = response.size();
    char *responseCstr = new char[responseSize + 1];
    char *contentType = new char[128];

    strncpy(responseCstr, response.c_str(), responseSize);
    responseCstr[responseSize] = '\0';

    char *index = strstr(responseCstr, "HTTP/");
    if (index != nullptr) {
        sscanf(index, "%*s %d", &resp.statusCode);
    }

    index = strstr(responseCstr, "Content-Type:");
    if (index != nullptr) {
        sscanf(index, "%*s %s", contentType);
        resp.contentType = contentType;
    }

    index = strstr(responseCstr, "Content-Length:");
    if (index != nullptr) {
        sscanf(index, "%*s %ld", &resp.contentLength);
    }

    delete[] contentType;
    delete[] responseCstr;
    return resp;
}
