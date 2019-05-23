  section .multiboot_header
  align 8
header_start:
  dd 0xe85250d6
  dd 0
  dd header_end - header_start
  dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

  ;; some framebuffer info
;;   align 8
;; mb_framebuffer_info_begin:
;;   dw 5
;;   dw 0
;;   dd 20
;;   dd 40
;;   dd 24
;;   dd 1
;; mb_framebuffer_info_end:

  ;; acpi 2 header request
  align 8
mb_acpi_info_begin:
  dw 1
  dw 1
  dd 12
  dd 0xf
  dd 0
mb_acpi_info_end:
  ;; dw 6                          ; 12 bytes alignment

  dw 0                          ; type
  dw 0                          ; flags
  dd 8                          ; size
header_end:
