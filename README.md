# lmigtester

A VMM on KVM written in C++.

## Memory Layout

```
                        ~                             ~
            0xffff_ffff +-----------------------------+
                        ~                             ~
                        | INITRAMFS                   |
            0x0f00_0000 +-----------------------------+
                        ~                             ~
                        | PROTECTED-MODE LINUX KERNEL |
RIP->       0x0010_0000 +-----------------------------+ <-
                        ~                             ~  |
                        | MBBIOS                      |  |
            0x000f_0000 +-----------------------------+  |- I/O MEMORY HOLE
                        ~                             ~  |
                        | VGA                         |  |
            0x000a_0000 +-----------------------------+ <-
                        ~                             ~
                        | EBDA                        |
            0x0009_fc00 +-----------------------------+
                        ~                             ~
                        | BOOT PAGE TABLE(64)         |
            0x0003_0000 +-----------------------------+
                        ~                             ~
                        | COMMAND LINE                |
            0x0002_0000 +-----------------------------+
                        ~                             ~
                        | BOOT PARAMETERS             |
RSI->       0x0001_0000 +-----------------------------+
                        ~                             ~
                        | HEAP END                    |
                        ~                             ~
            0x0000_2000 +-----------------------------+
                        | ZERO PAGE                   |
            0x0000_1000 +-----------------------------+
                        | RESERVED                    |
            0x0000_0000 +-----------------------------+
```

## TODO

- [ ] Boot linux in 32-bit boot protocol
- [ ] Support COM1
- [ ] Write error handling logics in decent manner with exceptions
- [ ] Classes, members, methods, and functions also need to be designed properly
- [ ] Capacity checks
- [ ] Memory bank
- [ ] Unit test
- [ ] Support 64-bit boot protocol

## References

- https://docs.kernel.org/virt/kvm/index.html
- https://www.kernel.org/doc/html/latest/arch/x86/boot.html
- Intel® 64 and IA-32 Architectures Software Developer’s Manual Combined Volumes: 1, 2A, 2B, 2C, 2D, 3A, 3B, 3C, 3D, and 4
- https://github.com/bobuhiro11/gokvm
- https://github.com/kvmtool/kvmtool
