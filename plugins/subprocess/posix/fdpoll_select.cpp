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
#ifndef HAVE_EPOLL

#include "fdpoll.hpp"
#include "errno.hpp"

#include <algorithm>

#include <sys/select.h>
#include <sys/time.h>

namespace subprocess {
  fdpoll::fdpoll() { }
  fdpoll::~fdpoll() { }
  void fdpoll::add(int fd, unsigned what) {
    fds[fd] = what;
  }
  void fdpoll::remove(int fd) {
    fds.erase(fd);
  }
  std::vector< std::pair<int, unsigned> > fdpoll::wait(int timeout_ms) {
    fd_set rfds, wfds, efds;
    int nfds = 0;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    // TODO this is slow. maybe add a fdpoll::setup
    for(std::map<int, unsigned>::const_iterator i = fds.begin(); i != fds.end(); ++i) {
      nfds = std::max(i->first, nfds);
      if(flagset(fdpoll::read, i->second)) {
        FD_SET(i->first, &rfds);
      }
      if(flagset(fdpoll::write, i->second)) {
        FD_SET(i->first, &wfds);
      }
      if(flagset(fdpoll::error, i->second)) {
        FD_SET(i->first, &efds);
      }
    }
    timeval tv;
    if(timeout_ms != fdpoll::indefinitely) {
      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
    }
    int const retfds = select(nfds+1, &rfds, &wfds, &efds,
                              timeout_ms != fdpoll::indefinitely ? &tv : 0x0);
    if(retfds == -1) {
      int const errno_ = errno;
      throw subprocess::exception("select") << subprocess::errno_info(errno_);
    }
    else if(retfds == 0) { // timeout
      return std::vector< std::pair<int, unsigned> >();
    }

    std::vector< std::pair<int, unsigned> > ret;
    ret.reserve(retfds);
    for(std::map<int, unsigned>::const_iterator i = fds.begin(); i != fds.end(); ++i) {
      unsigned what = 0;
      if(flagset(fdpoll::read, i->second) && FD_ISSET(i->first, &rfds)) {
        what |= fdpoll::read;
      }
      if(flagset(fdpoll::write, i->second) && FD_ISSET(i->first, &wfds)) {
        what |= fdpoll::write;
      }
      if(flagset(fdpoll::error, i->second) && FD_ISSET(i->first, &efds)) {
        what |= fdpoll::error;
      }
      ret.push_back(std::make_pair(i->first, what));
    }
    return ret;
  }
}
#endif
