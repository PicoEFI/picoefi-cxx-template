#include <cstdint>
#include <cstddef>
#include <efi.h>

// Halt and catch fire function, defined below.
namespace {

[[noreturn]] void hcf();

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions in memory.cpp, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {
    int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
    void __cxa_pure_virtual() { hcf(); }
    void __cxa_deleted_virtual() { hcf(); }
    void *__dso_handle;
    // Not thread safe: replace before statics can be initialised concurrently.
    int __cxa_guard_acquire(std::uint64_t *guard) { return *reinterpret_cast<std::uint8_t *>(guard) == 0; }
    void __cxa_guard_release(std::uint64_t *guard) { *reinterpret_cast<std::uint8_t *>(guard) = 1; }
}

// Extern declarations for global constructors array.
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

namespace {

void hcf() {
    for (;;) {
#if defined (__x86_64__) || defined (__i386__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

// OutputString takes a non-const string, so the message is a writable array.
char16_t hello_message[] = u"Hello world\r\n";

}

extern "C" EFI_STATUS efi_main(EFI_HANDLE, EFI_SYSTEM_TABLE *SystemTable) {
    // Call global constructors.
    for (std::size_t i = 0; &__init_array_start[i] != __init_array_end; i++) {
        __init_array_start[i]();
    }

    SystemTable->ConOut->OutputString(SystemTable->ConOut, reinterpret_cast<EFI_CHAR16 *>(hello_message));
    hcf();
}
