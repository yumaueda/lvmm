#ifndef BASECLASS_HPP
#define BASECLASS_HPP


#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>


class BaseClass {
    public:
        explicit BaseClass(int fd);
        ~BaseClass();

        template<typename... kvmIoctlArgs>
        int kvmIoctl(unsigned long request, kvmIoctlArgs... args) {
            int r = ioctl(fd, request, args...);
            std::ostringstream args_oss;
            args_oss << fd << ',' << request;
            ((args_oss << ',' << args), ...);
            std::string args_str = args_oss.str();

#ifdef MONITOR_IOCTL
            std::cout << (std::string(__func__)\
                + ": ioctl("\
                + args_str\
                + "): "\
                + strerror(errno)) << std::endl;
#endif  // MONITOR_IOCTL

            if (r < 0) {
                r = -errno;
                throw std::runtime_error(std::string(__func__)\
                        + ": ioctl("\
                        + args_str\
                        + "): "\
                        + strerror(errno));
            }
            return r;
        }

    protected:
        int fd;
};


#endif  // BASECLASS_HPP
