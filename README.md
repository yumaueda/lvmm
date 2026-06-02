# lvmm

A VMM on KVM written in C++.

## Build & Run

The VMM needs three artifacts side-by-side at runtime: the `lvmm` binary,
a guest kernel image at `./bzImage`, and a `./initramfs` cpio archive.

### Host prerequisites

```
sudo apt-get install -y \
    build-essential libelf-dev libssl-dev libncurses-dev bison flex bc cpio
```

Go (for u-root) and a `/dev/kvm` accessible to your user (group `kvm` or an
ACL entry; check with `getfacl /dev/kvm`) are also required.

### lvmm binary

```
make lvmm
```

### Guest kernel (`bzImage`)

The bundled `linux.config` targets Linux 5.14.3. With a newer host gcc
you also need `WERROR=0` to silence `-Wuse-after-free` in `tools/objtool`.

```
curl -O https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.14.3.tar.xz
tar xf linux-5.14.3.tar.xz
cp linux.config linux-5.14.3/.config
cd linux-5.14.3
make olddefconfig
make WERROR=0 -j$(nproc) bzImage
cp arch/x86/boot/bzImage ../bzImage
```

### Initramfs

`scripts/geninitramfs.bash` drives u-root and bakes `gosh` in as
`/bbin/uinit` so PID 1 boots straight into a shell prompt.

```
go install github.com/u-root/u-root@latest
mkdir -p ~/go/src/github.com/u-root
git clone https://github.com/u-root/u-root ~/go/src/github.com/u-root/u-root
GOPATH=$HOME/go make initramfs
```

### Run

```
./lvmm
```

`./lvmm` switches the host terminal to raw mode and forwards keystrokes to
the guest serial port, so you can type commands at the `$` prompt.

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

## References

- https://www.kernel.org/doc/html/latest/virt/kvm/index.html
- https://www.kernel.org/doc/html/latest/arch/x86/boot.html
- Intel® 64 and IA-32 Architectures Software Developer’s Manual Combined Volumes: 1, 2A, 2B, 2C, 2D, 3A, 3B, 3C, 3D, and 4
- https://github.com/torvalds/linux
- https://github.com/bobuhiro11/gokvm
- https://github.com/kvmtool/kvmtool
- https://0xax.gitbooks.io/linux-insides/content/index.html
- https://wiki.osdev.org
