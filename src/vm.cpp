/*
 *  src/vm.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <vm.hpp>

#include <sys/mman.h>
#include <unistd.h>
#include <linux/kvm.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <exception>
#include <ios>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <thread>

#include <boot.hpp>
#include <cmos.hpp>
#include <paging.hpp>
#include <pci.hpp>
#include <pio.hpp>
#include <post.hpp>
#include <util.hpp>


int VM::setTSSAddr() {
    int r;

    r = kvmIoctl(KVM_SET_TSS_ADDR, TSS_BASE);

    if (r < 0) {
        perror(("VM::" + std::string(__func__) + ": kvmIoctl").c_str());
        return -errno;
    }

    std::cout << "VM::" << __func__ << ": registered" << std::endl;

    return r;
}

int VM::setIdentityMapAddr() {
    int r;

    r = kvmIoctl(KVM_SET_IDENTITY_MAP_ADDR, IDENTITY_MAP_BASE);

    if (r < 0) {
        perror(("VM::" + std::string(__func__) + ": kvmIoctl").c_str());
        return -errno;
    }

    std::cout << "VM::" << __func__ << ": registered" << std::endl;

    return r;
}

int VM::allocGuestRAM() {
    // not caring about hugetlbpage
    ram_start = mmap(NULL, vm_conf.ram_size, (PROT_READ|PROT_WRITE),
                        (MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);

    if (ram_start == MAP_FAILED) {
        perror(("VM::" + std::string(__func__) + ": mmap").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__ << ": VM.ram_start mmaped: "
        << ram_start << std::endl;

    if (madvise(ram_start, vm_conf.ram_size, MADV_MERGEABLE) < 0) {
        perror(("VM::" + std::string(__func__) + ": madvise").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__
        << ": VM.ram_start madvised as MERGEABLE: "
        << ram_start << std::endl;

    return 0;
}

int VM::setUserMemRegion() {
    // TMP implementation
    // We don't implement membank yet. So there's a limitation of ram_size!
    // RECOMMENDED: userspace_addr[0:20] == guest_phys_addr[0:20]
    int r;
    user_memory_region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = vm_conf.ram_size,
        .userspace_addr = reinterpret_cast<uint64_t>(ram_start),
    };

    r = kvmIoctl(KVM_SET_USER_MEMORY_REGION, &user_memory_region);

    if (r < 0) {
        perror(("VM::" + std::string(__func__) + ": kmvIoctl").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__ << ": registered" << std::endl;

    return r;
}

int VM::createVcpu() {
    int r;
    vcpus = reinterpret_cast<Vcpu*>(
                operator new[](vm_conf.vcpu_num*sizeof(Vcpu)));
    std::cout << "VM::vcpus: " << vcpus << std::endl;

    for (int i = 0; i < vm_conf.vcpu_num; i++) {
        r = kvmIoctl(KVM_CREATE_VCPU, i);
        if (r < 0) {
            perror(("VM::" + std::string(__func__) + ": kvmIoctl: ").c_str());
            return -errno;
        }
        std::cout << "VM::" << __func__ << ": "
            << "fd=" << r << " cpu_id= " << i << std::endl;
        new(&vcpus[i]) Vcpu(r, kvm, this, i);
        std::cout << "&VM.vcpus[" << i << "]: " << &vcpus[i] << std::endl;
    }

    return 0;
}

int VM::registerPIOHandler(uint16_t port_start, uint16_t port_end,
        PIOHandler in_func, PIOHandler out_func) {
    for (uint16_t i = port_start; i < port_end; ++i) {
        pio_handler[i][KVM_EXIT_IO_IN] = in_func;
        pio_handler[i][KVM_EXIT_IO_OUT] = out_func;
    }

    std::cout << "VM::" << __func__
        << ": port_start: " << port_start
        << ": port_end: "   << port_end
        << std::endl;

    return 0;
}

int VM::initPIOHandler() {
    // Fill pio_handler with default_pio_handler
    registerPIOHandler(0, PIO_PORT_NUM,
            default_pio_handler, default_pio_handler);
    // Alternate port 0xed based delay
    registerPIOHandler(PIO_PORT_ALT_DELAY_START, PIO_PORT_ALT_DELAY_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // COM4
    registerPIOHandler(PIO_PORT_COM4_START, PIO_PORT_COM4_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // COM2
    registerPIOHandler(PIO_PORT_COM2_START, PIO_PORT_COM2_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // VGA
    registerPIOHandler(PIO_PORT_VGA_0_START, PIO_PORT_VGA_0_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    registerPIOHandler(PIO_PORT_VGA_1_START, PIO_PORT_VGA_1_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // COM3
    registerPIOHandler(PIO_PORT_COM3_START, PIO_PORT_COM3_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // COM1 (tmp) FIXME: handler needed!!!
    registerPIOHandler(PIO_PORT_COM1_START, PIO_PORT_COM1_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    // Reset Generator
    registerPIOHandler(PIO_PORT_RST_GEN_START, PIO_PORT_RST_GEN_END,
            default_pio_handler, reset_generator_handler_out);
    // unknown...?
    registerPIOHandler(PIO_PORT_UNKNOWN_1_START, PIO_PORT_UNKNOWN_1_END,
            do_nothing_pio_handler, do_nothing_pio_handler);
    registerPIOHandler(PIO_PORT_UNKNOWN_2_START, PIO_PORT_UNKNOWN_2_END,
            do_nothing_pio_handler, do_nothing_pio_handler);

    // PCI configuration space access mechanism 2
    registerPIOHandler(PIO_PORT_PCI_CSAM2_START, PIO_PORT_PCI_CSAM2_END,
            do_nothing_pio_handler, do_nothing_pio_handler);

    // PS2 controller setting may be needed on WSL2?
    // TMP! FOR DEBUG PURPOSE
    registerPIOHandler(0x60, 0x70,
            do_nothing_pio_handler, do_nothing_pio_handler);

    // IO Devices
    //
    // unique_ptr is copied, but when VM.pio_handler is out of scope,
    // VM.iodev is also out, and vice versa.
    for (auto& e : iodev) {
        IODev* ptr = e.get();
        auto read_func = [ptr](uint16_t port, char* data_ptr, uint8_t size) {
            return ptr->Read(port, data_ptr, size);
        };
        auto write_func = [ptr](uint16_t port, char* data_ptr, uint8_t size) {
            return ptr->Write(port, data_ptr, size);
        };
        registerPIOHandler(e->port, e->port+e->size, read_func, write_func);
    }

    // PCI Devices
    // ...

    return 0;
}

void VM::addIODev(IODev* iodev_ptr) {
    iodev.emplace_back(iodev_ptr);
}

int VM::initMachine() {
    int r;

    addIODev(new Post);
    addIODev(new CMOS);

    for (const InitMachineFunc e : initmachine_func) {
        r = (this->*e)();
        if (r < 0)
            return r;
    }

    std::cout << "VM::" << __func__ << ": success" << std::endl;
    return 0;
}

int VM::createPageTable(uint64_t boot_pgtable_base) {
    char*    pagetable_start = reinterpret_cast<char*>(ram_start)
                                    + boot_pgtable_base;
    PTE*     pml4_start      = reinterpret_cast<PTE*>(pagetable_start);
    PTE*     pdpte_start     = reinterpret_cast<PTE*>(
                                    pagetable_start+PAGE_SIZE_4KB);
    PTE*     pde_start       = reinterpret_cast<PTE*>(
                                    pagetable_start+PAGE_SIZE_4KB*2);
    uint64_t pml4_start_gpa  = boot_pgtable_base;
    uint64_t pdpte_start_gpa = boot_pgtable_base+PAGE_SIZE_4KB;
    PTE      pml4e, pdpte, pde;

    assert(~PL4_ADDR_MASK && reinterpret_cast<uint64_t>(pagetable_start));

    // 6 pages (0x1000*6 = 0x6000 bytes) are used for the boot page table
    // to straight map 0-4GiB memory area.
    std::memset(pagetable_start, 0, BOOT_PAGETABLE_SIZE);

    // PML4E
    pml4e  = PAGE_FLAG_PCD | PAGE_FLAG_RW | PAGE_FLAG_P;
    pml4e += pml4_start_gpa + PAGE_SIZE_4KB;
    *pml4_start = pml4e;

    // PDPTE
    pdpte  = PAGE_FLAG_PCD | PAGE_FLAG_RW | PAGE_FLAG_P;
    pdpte += pdpte_start_gpa;
    for (int i = 0 ; i < BOOT_PDPTE_NUM; ++i) {
        pdpte += PAGE_SIZE_4KB;
        *(pdpte_start+i) = pdpte;
    }

    // PTE
    pde = PAGE_FLAG_G | PAGE_FLAG_PS | PAGE_FLAG_RW | PAGE_FLAG_P;
    for (int i = 0; i < BOOT_PDE_NUM; ++i) {
        *(pde_start+i) = pde;
        pde += PAGE_SIZE_2MB;
    }

    std::cout << "VM::" << __func__ << ": success" << std::endl;

    return 0;
}

int VM::initRAM(std::string cmdline) {
    ebda* ebda_start = reinterpret_cast<ebda*>((
                reinterpret_cast<uint8_t*>(ram_start)+EBDA_START));
    ebda* ebda_end;

    char* ramdisk_image = reinterpret_cast<char*>(ram_start)
                                + INITRAMFS_ADDR;
    char* kernel_image  = reinterpret_cast<char*>(ram_start)
                                + HIGHMEM_BASE;
    std::streamsize ramdisk_size, kernel_size;
    std::ios::pos_type kernel_load_offset;

    char* cmdline_start = reinterpret_cast<char*>(ram_start)
                                + COMMANDLINE_ADDR;
    char* cmdline_end;

    boot_params* boot_params_start = reinterpret_cast<boot_params*>((
                reinterpret_cast<uint8_t*>(ram_start)+BOOT_PARAMS_ADDR));
    boot_params* boot_params_end;

    ebda ebda_data = gen_ebda(vm_conf.vcpu_num);

    std::cout << "ebda generated" << '\n';
    std::cout << "ebda_data.fps.checksum: "
        << ebda_data.fps.checksum+0 << '\n';
    std::cout << "ebda_data.ctable.checksum: "
        << ebda_data.ctable.checksum+0 << std::endl;

    if (is_mp_checksum_valid(&ebda_data.fps)) {
        std::cout << "ebda_data.fps.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.fps.checksum corrupted" << std::endl;
    }

    if (is_mp_checksum_valid(&ebda_data.ctable)) {
        std::cout << "ebda_data.ctable.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.ctable.checksum corrupted" << std::endl;
    }

    ebda_end = std::copy_n(&ebda_data, 1, ebda_start);
    std::cout << "ebda_data copied to guest RAM: " << ebda_start << std::endl;
    std::cout << "ebda_end: " << ebda_end << std::endl;

    // initramfs
    ramdisk_size = get_ifs_size(initramfs);
    std::cout << "initramfs size: " << ramdisk_size << std::endl;
    if (!initramfs.read(ramdisk_image, ramdisk_size)) {
        std::cerr << "couldn't read from initramfs" << std::endl;
        return 1;
    }
    std::cout << "initramfs copied to guest RAM: "
        << static_cast<void*>(ramdisk_image) << std::endl;

    // cmdline
    // FIXME: should check commandline size before do this
    std::cout << "cmdline size: " << cmdline.size() << std::endl;
    cmdline_end = std::copy_n(cmdline.begin(), cmdline.size(), cmdline_start);
    *cmdline_end = '\0';  // null-terminate
    std::cout << "cmdline copied to guest RAM: "
        << static_cast<void*>(cmdline_start) << std::endl;
    std::cout << "cmdline_end: "
        << static_cast<void*>(cmdline_end) << std::endl;

    // bootparam
    boot_params bp;

    kernel.seekg(SETUP_HEADER_ADDR, std::ios::beg);
    kernel.read(reinterpret_cast<char*>(&bp.header), sizeof(bp.header));
    if (!kernel) {
        kernel.seekg(0, std::ios::beg);
        std::cerr << "Couldn't read a setup header "
            "from the kernel image" << std::endl;
        return 1;
    }
    kernel.seekg(0, std::ios::beg);
    std::cout << "bootparam setup header has been "
        "loaded from the kernel image" << std::endl;

    if (bp.header.is_valid()) {
        std::cout << "bootparam setup header is valid" << std::endl;
    } else {
        std::cerr << "bootparam setup header is invalid "
            "or the boot protocol version is old" << std::endl;
        return 1;
    }

    if (bp.header.check_setup_sects())
        std::cout << "The value of setup_sects has been "
            "modified to 4" << std::endl;

    std::cout << "Writing to bootparam..." << std::endl;

    bp.add_e820_entry(REALMODE_IVT_START,
            EBDA_START-REALMODE_IVT_START, BOOT_E820_TYPE_RAM);
    bp.add_e820_entry(EBDA_START,
            VGARAM_START-EBDA_START, BOOT_E820_TYPE_RESERVED);
    bp.add_e820_entry(MBBIOS_START, MBBIOS_SIZE, BOOT_E820_TYPE_RESERVED);
    bp.add_e820_entry(HIGHMEM_BASE,
            vm_conf.ram_size-HIGHMEM_BASE, BOOT_E820_TYPE_RAM);

    // change later to print these writing processs
    // some kind of setter?
    bp.header.vid_mode       = BOOT_HDR_VID_MODE_NML;
    bp.header.type_of_loader = BOOT_HDR_BLT_UNDEFINED;
    bp.header.loadflags      = BOOT_HDR_LF_HIGH
                             | BOOT_HDR_LF_KEEP_SGMT
                             | BOOT_HDR_LF_HEAP;
    bp.header.ramdisk_image  = INITRAMFS_ADDR;
    bp.header.ramdisk_size   = ramdisk_size;
    bp.header.heap_end_ptr   = BOOT_PARAMS_ADDR - BOOT_HDR_HEAPEND_OFFSET;
    bp.header.cmd_line_ptr   = COMMANDLINE_ADDR;

    boot_params_end = std::copy_n(&bp, 1, boot_params_start);
    std::cout << "boot_params copied to guest RAM: "
        << static_cast<void*>(boot_params_start) << std::endl;
    std::cout << "boot_params_end: "
        << static_cast<void*>(boot_params_end) << std::endl;

    // kernel
    assert(!vm_conf.is_64bit_boot);  // to be implemented
    kernel_load_offset = (bp.header.setup_sects+1) * SECT_SIZE;
    kernel.seekg(kernel_load_offset, std::ios::beg);
    kernel_size = get_ifs_size(kernel) - kernel_load_offset;
    if (!kernel.read(kernel_image, kernel_size)) {
        std::cerr << "couldn't load kernel image" << std::endl;
        return 1;
    }
    std::cout << "kernel image copied to guest RAM: "
        << static_cast<void*>(kernel_image) << std::endl;
    std::cout << "kernel load offset (setupsz): "
        << kernel_load_offset << std::endl;
    kernel.seekg(0, std::ios::beg);

    // boot page table
    if (vm_conf.is_64bit_boot) {
        createPageTable(BOOT_PAGETABLE_BASE);
        std::cout << "boot page table created at: "
            << BOOT_PAGETABLE_BASE << std::endl;
    }

    std::cout << "VM::" << __func__ << ": success" << std::endl;

    return 0;
}

int VM::initVcpuRegs() {
    for (int i = 0; i < vm_conf.vcpu_num; ++i) {
        if ((vcpus+i)->InitRegs(HIGHMEM_BASE, BOOT_PARAMS_ADDR))
            return 1;
    }
    std::cout << "VM::" << __func__ << ": success" << std::endl;
    return 0;
}

int VM::initVcpuSregs(bool is_64bit_boot) {
    assert(!is_64bit_boot);
    for (int i = 0; i < vm_conf.vcpu_num; ++i) {
        if ((vcpus+i)->InitSregs(is_64bit_boot))
            return 1;
    }
    std::cout << "VM::" << __func__ << ": success" << std::endl;
    return 0;
}

int VM::Boot() {
    std::vector<std::thread> threads;

    for (int i = 0; i < vm_conf.vcpu_num; ++i) {
        std::cout << "VM::" << __func__ << ": Booting vCPU["
            << i << "]" << std::endl;
        threads.emplace_back(&Vcpu::RunLoop, &vcpus[i]);
    }

    for (auto& e : threads) {
        if (e.joinable()) {
            e.join();
        }
    }

    return 0;
}

VM::VM(int vm_fd, KVM* kvm, vm_config vm_conf)\
        : BaseClass(vm_fd), kvm(kvm), vm_conf(vm_conf) {
    std::cout << "Constructing VM..." << std::endl;

    // Currently we only assumes the case where unrestricted_guest == 1.
    // So we won't call following APIs:
    // - KVM_SET_TSS
    // - KVM_SET_IDENTITY_MAP

    // Create VIOAPIC, VPIC. Set up future vCPUs to use them as a local APIC.
    // - GSI00-15 -> IOAPIC/PIC
    // - GIS16-23 -> IOAPIC
    kvmIoctlCtor(KVM_CREATE_IRQCHIP);
    std::cout << "IRQCHIP created" << std::endl;
    kvmIoctlCtor(KVM_CREATE_PIT2, &pit_config);
    std::cout << "PIT2 created" << std::endl;

    kernel.open(vm_conf.kernel_path, std::ios::in | std::ios::binary);
    initramfs.open(vm_conf.initramfs_path, std::ios::in | std::ios::binary);

    if (!kernel)
        throw std::runtime_error("Cloud not open the kernel: "
                + std::string(vm_conf.kernel_path));
    if (!initramfs)
        throw std::runtime_error("Cloud not open the initramfs: "
                + std::string(vm_conf.initramfs_path));

    if (is_kernel_elf(kernel))
        throw std::runtime_error("VM::"+std::string(__func__)+
                ": the kernel is elf file");
    std::cout << "Verified that the kernel is not an elf file" << std::endl;

    std::cout << "Constructed VM." << std::endl;
}

VM::~VM() {}
