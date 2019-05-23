#include "types.hpp"
#include "paging.hpp"
#include "interrupts.hpp"
#include "printf.hpp"
#include "ahci.hpp"
#include "ahci_utils.hpp"
#include "acpi.hpp"
#include "pci.hpp"

#define PF_ENTRY 14

uint64_t *p4_page_table_addr;

#define PAGE_TABLE_ENTRIES 512
#define MEM_TO_DISK_MAPPING_OFFSET 0xffffffff00000000
#define PAGE_SIZE 4096
#define DISK_PAGES 4

uint64_t p3_page_table_for_disk[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};
uint64_t p2_page_table_for_disk[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};
uint64_t p1_page_table_for_disk[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};

signed int current_page_mapped;

uint8_t disk_buffer[4096] __attribute__((aligned(4096)));

HBA_PORT *port_ptr_addr;

struct IDT_Pointer {
  unsigned short int size;
  uint64_t address;
} __attribute__((packed)) idt_ptr;

void init_interrupts(gate_entry *int_table, uint64_t *p4_page_table_, HBA_PORT *port_ptr) {
  p4_page_table_addr = p4_page_table_;
  port_ptr_addr = port_ptr;
  current_page_mapped = -1;
  uint64_t int_page_fault_addr = reinterpret_cast<uint64_t>(int_page_fault);
  int_table[PF_ENTRY].segment = 8;
  int_table[PF_ENTRY].offset_low = (uint16_t)(int_page_fault_addr & (0xffff));
  int_table[PF_ENTRY].offset_middle = (uint16_t)((int_page_fault_addr >> 16) & (0xffff));
  int_table[PF_ENTRY].offset_high = (uint32_t)((int_page_fault_addr >> 32) & (0xffffffff));
  int_table[PF_ENTRY].p = 1;
  int_table[PF_ENTRY].type = 0xe;

  // paging conf
  unsigned int p4_index = (MEM_TO_DISK_MAPPING_OFFSET & P4_ENTRY_MASK) >> P4_OFFSET;
  unsigned int p3_index = (MEM_TO_DISK_MAPPING_OFFSET & P3_ENTRY_MASK) >> P3_OFFSET;
  unsigned int p2_index = (MEM_TO_DISK_MAPPING_OFFSET & P2_ENTRY_MASK) >> P2_OFFSET;
  //unsigned int p1_index = (MEM_TO_DISK_MAPPING_OFFSET & P1_ENTRY_MASK) >> P1_OFFSET;

  uint64_t *p4_page_table = reinterpret_cast<uint64_t*>(&p4_page_table);
  p4_page_table_addr[p4_index] = reinterpret_cast<unsigned long long>(&p3_page_table_for_disk);
  p4_page_table_addr[p4_index] |= 0b11;
  p3_page_table_for_disk[p3_index] = reinterpret_cast<unsigned long long>(&p2_page_table_for_disk);
  p3_page_table_for_disk[p3_index] |= 0b11;
  p2_page_table_for_disk[p2_index] = reinterpret_cast<unsigned long long>(&p1_page_table_for_disk);
  p2_page_table_for_disk[p2_index] |= 0b11;
  //uint32_t short_int_table_addr = (uint32_t)((uint64_t)int_table & (0xffffffff));
  idt_ptr.size = 16*15;
  idt_ptr.address = reinterpret_cast<uint64_t>(int_table);
  __asm__("lidt [%0]\n" : : "r"(&idt_ptr));
}

struct interrupt_frame __attribute__((packed));

__attribute__((interrupt))
void int_page_fault(interrupt_frame *frame, uint64_t error_code) {
  //printf("page fault triggered\n");
  uint64_t faulty_page;
  __asm__ __volatile__("mov %0, cr2\n" : "=mr"(faulty_page));

  if(faulty_page >= MEM_TO_DISK_MAPPING_OFFSET && faulty_page < MEM_TO_DISK_MAPPING_OFFSET + DISK_PAGES*PAGE_SIZE) {
    if(current_page_mapped > -1) {
      // unmap old page
      //disk_op(port_ptr, current_page_mapped, (uint8_t*)MEM_TO_DISK_MAPPING_OFFSET + current_page_mapped*PAGE_SIZE, WRITE);
      disk_op(port_ptr_addr, current_page_mapped, disk_buffer, WRITE);

      p1_page_table_for_disk[current_page_mapped] = 0;
    }
    current_page_mapped = ((uint64_t)faulty_page - MEM_TO_DISK_MAPPING_OFFSET)/PAGE_SIZE;

    disk_op(port_ptr_addr, current_page_mapped, disk_buffer, READ);
    //unsigned int page_size = 4096;

    p1_page_table_for_disk[current_page_mapped] = reinterpret_cast<uint64_t>(disk_buffer);
    p1_page_table_for_disk[current_page_mapped] |= (1ULL << 0) | (1ULL << 1) | (1ULL << 7);
  }
}
