#ifndef PAGING_HPP
#define PAGING_HPP


using PTE = uint64_t;

constexpr uint64_t PAGE_FLAG_P         = 1;
constexpr uint64_t PAGE_FLAG_RW        = 1 << 1;
constexpr uint64_t PAGE_FLAG_US        = 1 << 2;
constexpr uint64_t PAGE_FLAG_PWT       = 1 << 3;
constexpr uint64_t PAGE_FLAG_PCD       = 1 << 4;
constexpr uint64_t PAGE_FLAG_A         = 1 << 5;
constexpr uint64_t PAGE_FLAG_D         = 1 << 6;
constexpr uint64_t PAGE_FLAG_PS        = 1 << 7;
constexpr uint64_t PAGE_FLAG_G         = 1 << 8;
constexpr uint64_t PAGE_FLAG_R         = 1 << 11;
constexpr uint64_t PAGE_FLAG_PAT       = 1 << 12;

constexpr uint32_t PAGE_SHIFT_4KB      = 12;
constexpr uint32_t PAGE_SHIFT_2MB      = 21;
constexpr uint32_t PAGE_SIZE_4KB       = (2<<(PAGE_SHIFT_4KB-1));
constexpr uint32_t PAGE_SIZE_2MB       = (2<<(PAGE_SHIFT_2MB-1));
constexpr uint32_t BOOT_PAGETABLE_SIZE = PAGE_SIZE_4KB*6;

constexpr int32_t  BOOT_PDPTE_NUM      = 4;
constexpr int32_t  BOOT_PDE_NUM        = 2048;
constexpr uint64_t PL4_ADDR_MASK       = 0x0000'FFFF'FFFF'F000;


#endif  // PAGING_HPP
