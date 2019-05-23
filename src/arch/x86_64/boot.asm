  extern init
  global start

  section .text
  bits 32
start:
  mov esp, stack_bottom
  mov ebp, stack_bottom
  push ebx
  push eax

  call init
  hlt

  section .bss
  align 4096
stack_top:
  resb 0x100000
stack_bottom:
  resb 0x4
  ;; resb 4                        ; empty space, not needed really
;; multiboot_header:
;;   resb 4
