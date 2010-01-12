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
#ifndef FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_FDPOLL_HPP
#define FLUSSPFERD_PLUGIN_SUBPROCESS_POSIX_FDPOLL_HPP

#ifndef HAVE_POSIX
#error "POSIX support required"
#endif

#include <vector>
#include <utility>
#ifndef HAVE_EPOLL
#include <map>
#endif

namespace subprocess {
  class fdpoll {
#ifdef HAVE_EPOLL
    int epollfd;
#else // select
    std::map<int, unsigned> fds;
#endif
  public:
    fdpoll();
    ~fdpoll();

    enum { read = 1, write = 2, error = 4 };
    void add(int fd, unsigned what);
    void remove(int fd);
    static int const indefinitely = -1;
    std::vector< std::pair<int, unsigned> > wait(int timeout_ms = indefinitely);
  };

  inline bool flagset(unsigned f, unsigned in) {
    return (in & f) == f;
  }
}

#endif
