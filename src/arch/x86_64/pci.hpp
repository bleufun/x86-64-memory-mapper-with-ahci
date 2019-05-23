#ifndef __PCI_HPP
#define __PCI_HPP

struct PCIConfSpaceHeader {
  unsigned short int vendor_id;
  unsigned short int device_id;

  unsigned short int command;
  unsigned short int status;

  unsigned char revision_id;
  unsigned char prog_if;
  unsigned char subclass;
  unsigned char class_code;

  unsigned char cache_line_size;
  unsigned char timer;
  unsigned char header_type;
  unsigned char bist;

  unsigned int bar[6];

  unsigned int cis_pointer;

  unsigned short int subsystem_vendor_id;
  unsigned short int subsystem_id;

  unsigned int expansion_rom_base_addr;

  unsigned char capabilities_pointer; // what
  unsigned char _reserved[7];
  unsigned char interrupt_line;
  unsigned char interrupt_pin;
  unsigned char min_grant;
  unsigned char max_latency;
} __attribute__((packed));

#endif
