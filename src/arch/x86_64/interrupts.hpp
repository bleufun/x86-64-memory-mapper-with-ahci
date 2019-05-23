#ifndef __INTERRUPTS_HPP
#define __INTERRUPTS_HPP

#include "types.hpp"
#include "ahci.hpp"

struct gate_entry {
  uint16_t offset_low;
  uint16_t segment;
  uint16_t ist : 3, zeros0 : 5, type : 5, dpl : 2, p : 1;
  uint16_t offset_middle;
  uint32_t offset_high;
  uint32_t zeros1;
} __attribute__((packed));

struct interrupt_frame;
void init_interrupts(gate_entry *int_table, uint64_t *p4_page_table_addr, HBA_PORT *port_ptr);
void int_page_fault(interrupt_frame *frame, uint64_t error_code);

#endif
