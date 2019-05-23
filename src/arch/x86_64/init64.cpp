#ifndef __INIT_64_CPP
#define __INIT_64_CPP

asm(".code64");

#include <cstring>

#include "config.hpp"
#include "paging.hpp"

#include "multiboot2.h"
#include "printf.hpp"

#include "types.hpp"
#include "interrupts.hpp"
#include "multiboot_utils.hpp"
#include "acpi.hpp"
#include "pci.hpp"
#include "ahci.hpp"
#include "ahci_utils.hpp"

#define PAGE_TABLE_ENTRIES 512

extern "C" void __stack_chk_fail() {
  // nothing
}

gate_entry interrupt_table[256];

static uint64_t *p4_page_table;
HBA_PORT *port_ptr;

extern "C" void init64(unsigned int multiboot_header, unsigned int page_table_ptr) {

  // zero segment registers
  __asm__("mov ax, 0\n mov ss, ax\n mov ds, ax\n mov es, ax\n mov gs, ax\n mov fs, ax\n" : : : "eax");

  // vga cursor position
  xpos = 0;
  ypos = 0;

  clean_vga_buffer(0xf4);

  p4_page_table = reinterpret_cast<uint64_t*>(page_table_ptr);
  printf("switching to long mode, page table address is 0x%x\n", p4_page_table);

  uint64_t acpi_address;
  read_multiboot_header(multiboot_header, &acpi_address);

  ACPISDTHeader *mcfg_ptr;
  read_acpi(reinterpret_cast<RSDT*>(acpi_address), &mcfg_ptr);

  uint64_t pci_address;
  HBA_MEM *abar_address;
  read_mcfg(mcfg_ptr, &pci_address, &abar_address);

  uint32_t port = find_sata_port(abar_address);
  PCIConfSpaceHeader *pci = reinterpret_cast<PCIConfSpaceHeader*>(pci_address);

  //bool pci_int_en = ((pci->command & (1 << 10)) == 0 ? true : false);
  // *port_offset = reinterpret_cast<((uint8_t*)abar_address + 0x100 + 0x80*port);
  HBA_PORT *port_ptr = reinterpret_cast<HBA_PORT*>((uint8_t*)(abar_address) + 0x100 + 0x80*port);
  //printf("Add. PCI info: int. disabled: %d\n", pci_int_en);
  printf("abar %x port %x\n", abar_address, port_ptr);
  //printf("HBA %b, clb: %x %x, fb: %x %x\n", port_ptr->cmd, port_ptr->clb, port_ptr->clbu, port_ptr->fb, port_ptr->fbu);

  /*
  uint8_t buffer[4096];
  for(unsigned int i = 0; i < 4096; i++) {
    buffer[i] = 0;
  }
  printf("reading...\n");
  disk_op(port_ptr, 0, buffer, READ);

  for(int i = 0; i < 4096; i++) {
    if(buffer[i] != 0) {
      printf("%x:%d ", i, buffer[i]);
      if(buffer[i] == 1) {
        // replace byte with another one
        buffer[i] = 2;
      }
    }
  }
  printf("\n");
  disk_op(port_ptr, 0, buffer, WRITE);
  */

  init_interrupts(interrupt_table, reinterpret_cast<uint64_t*>(p4_page_table), port_ptr);

  int *whatever = (int*)(0xffffffff00000000);
  for(int i = 0; i < 4; i++) {
    printf("%d...\n", i);
    *((uint8_t*)whatever + 4096*i) = 0xff;
  }
  //printf("%x\n", *whatever);
  //printf("mapped!\n");

  printf("done!\n");
  asm("hlt\n");

  return;
}

#endif
