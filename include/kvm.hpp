#ifndef KVM_HPP
#define KVM_HPP


#include <cstring>
#include <sstream>
#include <sys/ioctl.h>

#include <linux/kvm.h>

#include <vm.hpp>


class VM;

class KVM {
    public:
        explicit KVM();
        ~KVM();

        template<typename... kvmIoctlArgs>
        int kvmIoctl(unsigned long request, kvmIoctlArgs... args) {
            int r = ioctl(fd, request, args...);
            //debug
            std::ostringstream args_oss;
            args_oss << fd << ',' << request;
            ((args_oss << ',' << args), ...);
            std::string args_str = args_oss.str();

#ifdef MONITOR_KVMFD_IOCTL
            std::cout << (std::string(__func__)\
                + ": ioctl("\
                + args_str\
                + "): "\
                + strerror(errno)) << std::endl;
#endif  // MONITOR_KVMFD_IOCTL

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

        int kvmCreateVM(VM** ptr_vm, uint64_t ram_size, int vcpu_num);

    private:
        static constexpr const char* dev_kvm = "/dev/kvm";
        int fd;

        void kvmCapCheck();


        // KVM state
        int api_ver;

        // CAP
        int immediate_exit;
        int nr_slots, nr_as;
        int soft_vcpus_limit, hard_vcpus_limit;
        int mmap_size;
        int coalesced_mmio;
};

#endif  // KVM_HPP
