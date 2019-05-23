#include "acpi.hpp"
#include "multiboot2.h"
#include "printf.hpp"
#include "multiboot_utils.hpp"

void read_multiboot_header(unsigned long long multiboot_header, unsigned long long *acpi_address) {
  struct multiboot_tag *tag;
  unsigned size;
  size = *(unsigned *) multiboot_header;
  for (tag = (struct multiboot_tag *) (multiboot_header + 8);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                       + ((tag->size + 7) & ~7)))
    {
      //printf ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
      switch(tag->type) {
      case MULTIBOOT_TAG_TYPE_ACPI_OLD:
        {
          multiboot_tag_old_acpi *acpi_tag = reinterpret_cast<multiboot_tag_old_acpi*>(tag);
          RSDPDescriptor *desc = reinterpret_cast<RSDPDescriptor*>(acpi_tag->rsdp);
          (*acpi_address) = desc->RsdtAddress;
          break;
        }
      case MULTIBOOT_TAG_TYPE_ACPI_NEW:
        {
          printf("Error: Detected newer ACPI type, but this code can parse only ACPI 1.0 header\nShutting down...");
          asm("hlt\n");
          break;
        }
      }
    }
  //print(end_alert, sizeof(end_alert), 0, y);
}
