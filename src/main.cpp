#include <cstddef>
#include <cstdint>
#include <efi.h>

// Halt and catch fire function, defined below.
namespace {

[[noreturn]] void hcf();

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature). Like the memory functions in
// memory.cpp, these stubs can be moved to a different .cpp file, but should
// not be removed, unless you know what you are doing.
extern "C" {

int __cxa_atexit(void (*)(void *), void *, void *) {
    return 0;
}

void __cxa_pure_virtual() {
    hcf();
}

void __cxa_deleted_virtual() {
    hcf();
}

void *__dso_handle;

// Not thread safe: replace before statics can be initialised concurrently.
int __cxa_guard_acquire(std::uint64_t *guard) {
    return *reinterpret_cast<std::uint8_t *>(guard) == 0;
}

void __cxa_guard_release(std::uint64_t *guard) {
    *reinterpret_cast<std::uint8_t *>(guard) = 1;
}

}

// Extern declarations for global constructors array.
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

// The same for .ctors, which Clang 10 and older emit for x86 and which runs
// backwards.
extern void (*__ctors_start[])();
extern void (*__ctors_end[])();

// Halt and catch fire function.
namespace {

void hcf() {
    for (;;) {
#if defined(__x86_64__) || defined(__i386__)
        asm ("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm ("wfi");
#elif defined(__loongarch64)
        asm ("idle 0");
#endif
    }
}

// output_string() takes a non-const string, so the message is a writable
// array.
char16_t message[] = u"\r\n"
                     u"Hello world\r\n"
                     u"Press any key to return to firmware...";

}

extern "C" efi_status efi_main(efi_handle, efi_system_table *system_table) {
    // Call global constructors.
    for (void (**ctor)() = __init_array_start; ctor < __init_array_end; ) {
        (*ctor++)();
    }
    for (void (**ctor)() = __ctors_end; ctor > __ctors_start; ) {
        (*--ctor)();
    }

    // Print a message and a prompt to the firmware's console, after a newline,
    // as some firmware does not end its own messages with one.
    system_table->con_out->output_string(
        system_table->con_out, reinterpret_cast<std::uint16_t *>(message)
    );

    // Disable the 5-minute watchdog timer that the boot manager arms before
    // starting the program, which would reset the system during a long wait.
    system_table->boot_services->set_watchdog_timer(0, 0, 0, nullptr);

    // Wait for a key press, ignoring any made before the prompt, then return
    // to the firmware.
    system_table->con_in->reset(system_table->con_in, false);
    efi_input_key key;
    std::size_t index;
    while (system_table->con_in->read_key_stroke(system_table->con_in, &key) == EFI_NOT_READY) {
        system_table->boot_services->wait_for_event(
            1, &system_table->con_in->wait_for_key, &index
        );
    }

    return EFI_SUCCESS;
}
