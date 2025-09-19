#include "common.h"
#include "channel.h"
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

RequestChannel::RequestChannel() : rfd(-1), wfd(-1) {}

ssize_t RequestChannel::cread(void* msgbuf, int msgsize)  {
    // TODO: use read() to read msgsize bytes into msgbuf
    return 0;
}

ssize_t RequestChannel::cwrite(void* msgbuf, int msgsize)  {
    // TODO: use write() to write msgsize bytes into msgbuf
    return 0;
}

Response RequestChannel::send_request(const Request& req) {
    // TODO: use cwrite() and cread() to send a Request and return a Response
    Response resp;
    return resp;
}

Request RequestChannel::receive_request() {
    Request req;
    // TODO: use cread() to read into req
    return req;
}

bool RequestChannel::send_response(const Response& resp) {
    // TODO: use cwrite() to send resp
    return true;
}

PipeChannel::PipeChannel(int _rfd, int _wfd) {
    // TODO: complete the PipeChannel constructor
}

PipeChannel::~PipeChannel() {
    // TODO: complete the PipeChannel destructor
}

int FIFOChannel::open_pipe(std::string _pipe_name, int mode) {
    // TODO: use mkfifo() to create a fifo with r/w permissions and open() to open the pipe with "mode" flags
    return 0;
}

FIFOChannel::FIFOChannel(const std::string& name, const Side side) : 
    RequestChannel() {
    // TODO: complete the FIFOChannel constructor. the names for the pipes are given as pipe1 and pipe2
    pipe1 = "fifo_" + name + "1";
    pipe2 = "fifo_" + name + "2";
}

FIFOChannel::~FIFOChannel() {
    // TODO: complete the FIFOChannel destructor
}