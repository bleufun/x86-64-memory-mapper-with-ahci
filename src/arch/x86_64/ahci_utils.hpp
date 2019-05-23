#ifndef __AHCI_UTILS_HPP
#define __AHCI_UTILS_HPP

#define SECTOR_SIZE 512
#define SECTORS_PER_PAGE 8

#define WRITE false
#define READ true

uint32_t find_sata_port(HBA_MEM *abar_ptr);
//bool read(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf);
void disk_op(HBA_PORT *port, uint64_t start, uint8_t *buf, bool read_flag);

#endif
