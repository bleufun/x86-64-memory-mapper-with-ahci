//#include "init64.cpp"

asm(".code32");
//asm(".intel_syntax");

//#define VGA_MODE_WIDTH 80
//#define VGA_MODE_HEIGHT 24
// TODO rename dis
//#define video (unsigned char*)0xb8000
//#define LINES 24
//#define ATTRIBUTE 0xf4
//#define COLUMNS 80

//#include "multiboot2.h"

//static int xpos;
//static int ypos;

//#include "printf.hpp"

extern "C" void init(unsigned int magic, unsigned int _multiboot_header);
extern "C" void init64(unsigned int multiboot_header);

#define PAGE_TABLE_ENTRIES 512
/*
  .bss section starts after text
  we throw temporary stack here
*/
unsigned int multiboot_header;
int grub_magic;

// reserved space for our identity page map
unsigned long long p4_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};
unsigned long long p3_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};
unsigned long long p2_page_table[4][PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))); // = {0};
//unsigned long long p2_page_table_2[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};
//unsigned long long p2_page_table_3[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096))) = {0};

unsigned long long gdt_table[2] = {0};

struct GDT_Pointer {
  unsigned short int size;
  unsigned long long address;
} __attribute__((packed)) gdt_pointer;

void set_identity_mapping() {
  p4_page_table[0] = reinterpret_cast<unsigned long long>(&p3_page_table);
  p4_page_table[0] |= 0b11;

  /*
  p3_page_table[0] = reinterpret_cast<unsigned long long>(&p2_page_table);
  p3_page_table[0] |= 0b11;

  p3_page_table[1] = reinterpret_cast<unsigned long long>(&p2_page_table_2);
  p3_page_table[1] |= 0b11;

  p3_page_table[2] = reinterpret_cast<unsigned long long>(&p2_page_table_3);
  p3_page_table[2] |= 0b11;
  */
  for(int i = 0; i < 4; i++) {
    p3_page_table[i] = reinterpret_cast<unsigned long long>(&p2_page_table[i]);
    p3_page_table[i] |= 0b11;
  }

  const int page_size = 0x200000; // 2MiB
  for(unsigned int pi = 0; pi < 2; pi++) {
    for(unsigned int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
      p2_page_table[pi][i] = page_size*(i + PAGE_TABLE_ENTRIES*pi);
      p2_page_table[pi][i] |= (1ULL << 0) | (1ULL << 1) | (1ULL << 7);
    }
  }

  for(unsigned int pi = 2; pi < 4; pi++) {
    for(unsigned int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
      p2_page_table[pi][i] = page_size*(i + PAGE_TABLE_ENTRIES*pi);
      p2_page_table[pi][i] |= (1ULL << 0) | (1ULL << 1) | (1ULL << 7) | (1ULL << 4);
    }
  }

  __asm__("mov cr3, %0" : : "mr"(&p4_page_table));

  __asm__("mov eax, cr4\n"
          "or eax, 1 << 5\n"
          "mov cr4, eax\n"
          : : :"eax");

  __asm__("mov ecx, 0xc0000080\n"
          "rdmsr\n"
          "or eax, 1 << 8\n"
          "wrmsr\n"
          : : : "eax", "ecx");

  __asm__("mov eax, cr0\n"
          "or eax, 1 << 31\n"
          "mov cr0, eax\n"
          : : : "eax");

  return;
}

void set_gdt() {
  gdt_table[1] =
    (1ULL << 43) |
    (1ULL << 44) |
    (1ULL << 47) |
    (1ULL << 53) |
    (15ULL << 48) |
    (1ULL << 55); // code segment
  //gdt_table[2] = (1ULL << 41) | (1ULL << 44) | (1ULL << 47); // data segment
  gdt_pointer.size = 24 - 1 - 8;
  gdt_pointer.address = reinterpret_cast<unsigned long long>(&gdt_table);

  __asm__("lgdt [%0]\n" : : "r"(&gdt_pointer));
}

void init(unsigned int magic, unsigned int header) {
  //__asm__ __volatile__("mov %0, eax\n mov %1, ebx" : "=m"(grub_magic_var), "=m"(multiboot_header));
  //__asm__ __volatile__("lea esp, %0\n lea ebp, %0" : : "m"(stack[1023]));
  grub_magic = magic;
  multiboot_header = header;

  // TODO
  // check long mode availability

  // set identity mapping
  set_identity_mapping();

  // set new GDT
  set_gdt();

  // make far jump to 64bit code
  //init64(multiboot_header);
  __asm__("mov edi, %0\n"
          "mov esi, %1\n"
          "jmp 0x8:init64\n"
          : : "r"(multiboot_header), "r"(&p4_page_table));
  asm("hlt\n");
  return;
}
