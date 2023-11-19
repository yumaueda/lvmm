#ifndef IOCTL_HPP
#define IOCTL_HPP


template<typename... ioctlArgs>
int ioctlHelper(int fd, unsigned long request, ioctlArgs... args) {
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


#endif  // IOCTL_HPP
