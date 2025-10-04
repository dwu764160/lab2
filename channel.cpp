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
    if (msgbuf == nullptr || msgsize <= 0 || rfd < 0) return -1;
    char *buf = static_cast<char*>(msgbuf);
    ssize_t total = 0;
    while (total < msgsize) {
        ssize_t n = read(rfd, buf + total, msgsize - total);
        if (n > 0) {
            total += n;
        } else if (n == 0) {
            // EOF
            break;
        } else {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    return total;
}

ssize_t RequestChannel::cwrite(void* msgbuf, int msgsize)  {
    // TODO: use write() to write msgsize bytes into msgbuf
    if (msgbuf == nullptr || msgsize <= 0 || wfd < 0) return -1;
    char *buf = static_cast<char*>(msgbuf);
    ssize_t total = 0;
    while (total < msgsize) {
        ssize_t n = write(wfd, buf + total, msgsize - total);
        if (n >= 0) {
            total += n;
        } else {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    return total;
}

Response RequestChannel::send_request(const Request& req) {
    // TODO: use cwrite() and cread() to send a Request and return a Response
    Response resp;
    ssize_t w = cwrite((void*)&req, sizeof(Request));
    if (w != (ssize_t)sizeof(Request)) {
        // failed to send request
        return resp;
    }
    ssize_t r = cread((void*)&resp, sizeof(Response));
    if (r != (ssize_t)sizeof(Response)) {
        // failed to read full response
        Response empty;
        return empty;
    }
    return resp;
}

Request RequestChannel::receive_request() {
    Request req;
    // TODO: use cread() to read into req
    ssize_t r = cread((void*)&req, sizeof(Request));
    if (r != (ssize_t)sizeof(Request)) {
        // indicate failure
        // set type to FAILURE (enum in common.h)
        req.type = FAILURE;
    }
    return req;
}

bool RequestChannel::send_response(const Response& resp) {
    // TODO: use cwrite() to send resp
    ssize_t w = cwrite((void*)&resp, sizeof(Response));
    return (w == (ssize_t)sizeof(Response));
}

PipeChannel::PipeChannel(int _rfd, int _wfd) {
    // TODO: complete the PipeChannel constructor
    rfd = _rfd;
    wfd = _wfd;
}

PipeChannel::~PipeChannel() {
    // TODO: complete the PipeChannel destructor
    if (rfd >= 0) {
        close(rfd);
        rfd = -1;
    }
    if (wfd >= 0) {
        close(wfd);
        wfd = -1;
    }
}

int FIFOChannel::open_pipe(std::string _pipe_name, int mode) {
    // TODO: use mkfifo() to create a fifo with r/w permissions and open() to open the pipe with "mode" flags
    if (mkfifo(_pipe_name.c_str(), 0666) < 0) {
        if (errno != EEXIST) {
            perror(("mkfifo " + _pipe_name).c_str());
            return -1;
        }
    }
    int fd = open(_pipe_name.c_str(), mode);
    if (fd < 0) {
        perror(("open " + _pipe_name).c_str());
    }
    return fd;
}

FIFOChannel::FIFOChannel(const std::string& name, const Side side) : 
    RequestChannel() {
    // TODO: complete the FIFOChannel constructor. the names for the pipes are given as pipe1 and pipe2
    pipe1 = "fifo_" + name + "1";
    pipe2 = "fifo_" + name + "2";

    if (side == CLIENT_SIDE) {
        // client reads from pipe1, writes to pipe2
        rfd = open_pipe(pipe1, O_RDONLY);
        wfd = open_pipe(pipe2, O_WRONLY);
    } else {
        // server reads from pipe2, writes to pipe1
        rfd = open_pipe(pipe2, O_RDONLY);
        wfd = open_pipe(pipe1, O_WRONLY);
    }
}

FIFOChannel::~FIFOChannel() {
    // TODO: complete the FIFOChannel destructor
    if (rfd >= 0) {
        close(rfd);
        rfd = -1;
    }
    if (wfd >= 0) {
        close(wfd);
        wfd = -1;
    }
    if (!pipe1.empty()) unlink(pipe1.c_str());
    if (!pipe2.empty()) unlink(pipe2.c_str());
}