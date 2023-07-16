section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt

idt_load:
      push ebp
      mov ebp, esp          ; move stack pointer to the base pointer

      mov ebx, [ebp + 8]    ; + 8 points to the 1st argument passed to the function
      lidt [ebx]            ; load interrupt descriptor table
      pop ebp
      ret

int21h:
    cli                 ; stop interrupts
    pushad              ; store state
    call int21h_handler
    popad               ; restore state
    sti                 ; start interrupts
    iret

no_interrupt:
    cli                         ; stop interrupts
    pushad                      ; store state
    call no_interrupt_handler
    popad                       ; restore state
    sti                         ; start interrupts
    iret