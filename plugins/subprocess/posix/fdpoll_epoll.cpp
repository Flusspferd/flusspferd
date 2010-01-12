/*
The MIT License

Copyright (c) 2010 Flusspferd contributors (see "CONTRIBUTORS" or
                                     http://flusspferd.org/contributors.txt)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifdef HAVE_EPOLL

#include "fdpoll.hpp"
#include "errno.hpp"

#include <cstring>

#include <sys/epoll.h>

namespace subprocess {
  fdpoll::fdpoll() {
    epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if(epollfd == -1) {
      int const errno_ = errno;
      throw subprocess::exception("epoll_create") << subprocess::errno_info(errno_);
    }
  }
  fdpoll::~fdpoll() {
    close(epollfd);
  }

  void fdpoll::add(int fd, unsigned what) {
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    if(flagset(fdpoll::read, what)) {
      ev.events |= EPOLLIN;
    }
    if(flagset(fdpoll::write, what)) {
      ev.events |= EPOLLOUT;
    }
    // always waits for EPOLLERR so no need to check/set
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      int const errno_ = errno;
      throw subprocess::exception("epoll_ctl") << subprocess::errno_info(errno_);
    }
  }

  void fdpoll::remove(int fd) {
    epoll_event ev;
    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
      int const errno_ = errno;
      throw subprocess::exception("epoll_ctl") << subprocess::errno_info(errno_);
    }
  }

  std::vector< std::pair<int, unsigned> > fdpoll::wait(int timeout_ms) {
    enum { EVENTS_N = 8 };
    epoll_event events[EVENTS_N];
    int nfds = epoll_wait(epollfd, events, EVENTS_N, timeout_ms);
    if(nfds == -1) {
      int const errno_ = errno;
      throw subprocess::exception("epoll_wait") << subprocess::errno_info(errno_);
    }
    std::vector< std::pair<int, unsigned> > ret;
    ret.reserve(nfds);
    for(int i = 0; i < nfds; ++i) {
      unsigned what = 0;
      if(flagset(EPOLLIN, events[i].events)) {
        what |= fdpoll::read;
      }
      if(flagset(EPOLLOUT, events[i].events)) {
        what |= fdpoll::write;
      }
      if(flagset(EPOLLERR, events[i].events)) {
        what |= fdpoll::error;
      }
      ret.push_back( std::make_pair(events[i].data.fd, what) );
    }
    return ret;
  }
}

#endif
