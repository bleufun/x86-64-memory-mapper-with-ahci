#include "types.hpp"
#include "config.hpp"
#include "paging.hpp"
#include "acpi.hpp"
#include "pci.hpp"
#include "ahci.hpp"
#include "printf.hpp"
#include <cstring>

void read_acpi(RSDT *header, ACPISDTHeader **mcfg) {
  printf("ACPI 1.0 RSDT header, len %x, rev %x ", header->h.Length, header->h.Revision);
  prints(header->h.Signature, 4);
  printf("\n");

  int length = (header->h.Length - sizeof(header->h))/4;
  for(int i = 0; i < length; i++) {
    ACPISDTHeader *ptr = reinterpret_cast<ACPISDTHeader*>(header->PointerToOtherSDT[i]);
    if(strncmp(ptr->Signature, "MCFG", 4) == 0) {
      printf("MCFG header found\n");
      (*mcfg) = ptr;
      //read_mcfg(ptr);
    }
  }
}

bool scan_pci_devices_for_sata(uint64_t pci_address, HBA_MEM **abar_address) {
  bool found = false;
  unsigned int dev_count = 0;
  for(unsigned long long bus = 0; bus < 256; bus++) {
    for(unsigned long long dev = 0; dev < 32; dev++) {
      for(unsigned long long func = 0; func < 8; func++) {
        unsigned short int volatile *pci_enh_conf_addr = (unsigned short int volatile *)(pci_address | (bus << 20) | (dev << 15) | (func << 12));

        if(*pci_enh_conf_addr != 0x0 && *pci_enh_conf_addr != 0xffff) {
          dev_count++;
          PCIConfSpaceHeader *config = (PCIConfSpaceHeader*)(pci_enh_conf_addr);
          //printf("dev %d func %d %x:%x, type %x\n", dev, func, config->vendor_id, config->device_id, config->header_type);
          /*
          // list available BARs
          for(int i = 0; i < 6; i++) {
          if(config->bar[i] != 0)
          printf(":: 0x%x\n", config->bar[i]);
          }
          */

          if(config->vendor_id == SATA_VENDOR && config->device_id == SATA_DEVICE) {
            (*abar_address) = reinterpret_cast<HBA_MEM*>(config->bar[5] & (~0b1111)); // 4 lowest bits are flags (from AHCI 1.1 spec)
            found = true;
            printf("dev %d func %d %x:%x, type %x, ", dev, func, config->vendor_id, config->device_id, config->header_type);
            printf("AHCI interface st:%b cr:%b\n", config->status, config->command);
            //printf("add. info: cls: %d\n", config->cache_line_size);
          }
        }
      }
    }
  }
  printf("%d PCI device functions total\n", dev_count);
  return found;
}

void read_mcfg(ACPISDTHeader *mcfg, uint64_t *pci_address, HBA_MEM **abar_address) {
  uint32_t length = (mcfg->Length - sizeof(ACPISDTHeader) - 8)/16;

  printf("Scanning available PCI devices...\n");
  //printf("%x %d\n", mcfg, mcfg->Length);
  for(uint32_t i = 0; i < length; i++) {
    MCFGTableEntry *mcfgEntry = reinterpret_cast<MCFGTableEntry*>((char*)mcfg + 44);

    uint64_t temp_pci_address = mcfgEntry->address;
    HBA_MEM *temp_abar_address;
    bool found = scan_pci_devices_for_sata(temp_pci_address, &temp_abar_address);

    if(found) {
      printf("found SATA device\n");
      (*pci_address) = temp_pci_address;
      (*abar_address) = temp_abar_address;
      return;
    }
  }
}
