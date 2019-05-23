#ifndef __MULTIBOOT_UTILS_HPP
#define __MULTIBOOT_UTILS_HPP

void read_multiboot_header(unsigned long long multiboot_header, unsigned long long *acpi_address);

#endif
