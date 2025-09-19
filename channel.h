#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "common.h"
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

class RequestChannel {
protected:
    int rfd;
    int wfd;
    
    RequestChannel();

public:
    enum Side {SERVER_SIDE, CLIENT_SIDE};
    
    ~RequestChannel() {}
    
    ssize_t cread(void* msgbuf, int msgsize);
    ssize_t cwrite(void* msgbuf, int msgsize);

    Response send_request(const Request& req);

    Request receive_request();

    bool send_response(const Response& resp);
};

class PipeChannel : public RequestChannel {
public:
    PipeChannel(int _rfd, int _wfd);

    ~PipeChannel();
};

class FIFOChannel : public RequestChannel {
private:
    std::string pipe1, pipe2;

    int open_pipe(std::string _pipe_name, int mode);

public:
    FIFOChannel(const std::string& name, const Side _side);

    ~FIFOChannel();
};

#endif