section .asm

global idt_load
idt_load:
      push ebp
      mov ebp, esp      ; move stack pointer to the base pointer

      mov ebx, [ebp + 8] ; + 8 points to the 1st argument passed to the function
      lidt [ebx]   ; load interrupt descriptor table
      pop ebp
      ret