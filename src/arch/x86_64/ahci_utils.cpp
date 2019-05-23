//#include <cstring>
#include <cstring>

#include "printf.hpp"
#include "ahci.hpp"
#include "ahci_utils.hpp"

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier
 
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4
 
#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

static uint32_t check_type(HBA_PORT *port)
{
	unsigned int ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;
 
	switch (port->sig)
    {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
    }
}

uint32_t find_sata_port(HBA_MEM *abar_ptr) {
  //HBA_MEM *abar = abar_address;
  uint32_t port;
  uint32_t pi = abar_ptr->pi; // bitmap of available ports
  for(uint32_t i = 0; i < 32; i++) {
    if(pi & 1) {
      //HBA_PORT *port = reinterpret_cast<HBA_PORT*>(abar->ports[i]);
      int dt = check_type(&abar_ptr->ports[i]);

      switch(dt) {
      case AHCI_DEV_SATA:
        printf("found SATA drive port at %d, registering\n", i);
        port = i;
        break;
      case AHCI_DEV_SATAPI:
        printf("found SATAPI drive port (cdrom) at %d\n", i);
        break;
      default:
        break;
      }
    }
    pi >>= 1;
  }

  return port;
  //int *sata_addr = (int*)(pci_address | (31ULL << 15) | (2ULL << 12) + 0x20);

  //printf("pci dev: %x\n", *sata_addr);
}

int find_empty_slot(HBA_PORT *port) {
  uint32_t slots = (port->sact | port->ci);

  for(int i = 0; i < 32; i++) {
    if((slots & 1) == 0)
      return i;
    slots >>= 1;
  }

  return -1;
}

void disk_op(HBA_PORT *port, uint64_t start, uint8_t *buffer, bool read_flag = true) {
  // clear interrupt flags
  port->is = 0x0;

  int slot = find_empty_slot(port);

  if(slot == -1)
    printf("no slot found\n");

  HBA_CMD_HEADER *cmd = reinterpret_cast<HBA_CMD_HEADER*>(port->clb);

  cmd += slot;
  cmd->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // how big will it be?
  cmd->w = 0; // command direction - device -> host (why?)
  //cmd->prdtl = (uint16_t)((count - 1) >> 4) + 1; // 16 bits per entry?
  cmd->prdtl = 1;

  HBA_CMD_TBL *cmd_tbl = reinterpret_cast<HBA_CMD_TBL*>(cmd->ctba);
  // TODO write proper memset
  // manual memset
  for(unsigned long i = 0; i < sizeof(HBA_CMD_TBL) + (cmd->prdtl-1)*sizeof(HBA_PRDT_ENTRY); i++) {
    (*((uint8_t*)cmd_tbl + i)) = 0;
  }
  /*
    issuing only one PRD table entry, or only one continous read
  */
  cmd_tbl->prdt_entry[cmd->prdtl - 1].dba = reinterpret_cast<uint64_t>(buffer);
  cmd_tbl->prdt_entry[cmd->prdtl - 1].dbc = (1 << 12) - 1; // 4kB max
  cmd_tbl->prdt_entry[cmd->prdtl - 1].i = 0; // interrupt flag

  // command setup
  FIS_REG_H2D *cmd_fis = reinterpret_cast<FIS_REG_H2D*>(&cmd_tbl->cfis);

  cmd_fis->fis_type = FIS_TYPE_REG_H2D;
  cmd_fis->c = 1; // c = 1 - command
  if(read_flag == true)
    cmd_fis->command = ATA_CMD_READ_DMA_EX; // DMA read command
  else
    cmd_fis->command = ATA_CMD_WRITE_DMA_EX; // DMA read command

  start = SECTORS_PER_PAGE*start & (0xffffffffffff);
  cmd_fis->lba0 = (uint8_t)(start >> 0);
  cmd_fis->lba1 = (uint8_t)(start >> 8);
  cmd_fis->lba2 = (uint8_t)(start >> 16);
  cmd_fis->lba3 = (uint8_t)(start >> 24);
  cmd_fis->lba4 = (uint8_t)(start >> 32);
  cmd_fis->lba5 = (uint8_t)(start >> 40);

  cmd_fis->device = (1 << 6);
  //cmd_fis->count = (uint16_t)(SECTORS_PER_PAGE*count);
  cmd_fis->count = (uint16_t)SECTORS_PER_PAGE;

  // spin(lock)
  unsigned int spin = 0;
  while((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1e7)
    spin++;

  if(spin == 1e7)
    printf("Device is hung\n");

  port->ci = 1 << slot;

  while(true) {
    if((port->ci & (1 << slot)) == 0) {
      break;
    }
    //if(port->is & HBA_PxIS_TFES)
      //printf("error: TFES flag raised\n");
  }

  //if(port->is & HBA_PxIS_TFES)
    //printf("error: TFES flag raised\n");
}
