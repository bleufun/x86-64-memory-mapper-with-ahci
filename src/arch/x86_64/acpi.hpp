#ifndef __ACPI_HPP
#define __ACPI_HPP

#include "types.hpp"
#include "ahci.hpp"

// ACPI 1.0 RSDP
struct RSDPDescriptor {
  char Signature[8];
  char Checksum;
  char OEMID[6];
  char Revision;
  unsigned int RsdtAddress;
} __attribute__ ((packed));

// whats dis?
struct ACPISDTHeader {
  char Signature[4];
  unsigned int Length;
  unsigned char Revision;
  unsigned char Checksum;
  unsigned char OEMID[6];
  unsigned char OEMTableID[8];
  unsigned int OEMRevision;
  unsigned int CreatorID;
  unsigned int CreatorRevision;
} __attribute__ ((packed));

// ACPI MCFG table
struct MCFGTableEntry {
  unsigned long long address;
  unsigned short int group;
  unsigned char startPCI;
  unsigned char endPCI;
  unsigned int _reserved;
} __attribute__((packed));

// what was that?
struct RSDT {
  struct ACPISDTHeader h;
  unsigned int PointerToOtherSDT[];
} __attribute__ ((packed));

void read_acpi(RSDT *header, ACPISDTHeader **mcfg);
void read_mcfg(ACPISDTHeader *mcfg, uint64_t *pci_address, HBA_MEM **abar_address);

#endif
