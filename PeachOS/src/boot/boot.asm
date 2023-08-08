ORG 0x7c00
; 16 bit architecture is used
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; FAT16 Header
OEMIdentifier           db 'PEACHOS '
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'PEACHOS BOO'
SystemIDString          db 'FAT16   '


start:
    jmp 0:step2

step2:
    cli ; clear (disable) interrupts
    mov ax, 0x0 ; we can't set 0x7c0 directly to the ds & es registers
    mov ds, ax ; ds - data segment. By settings ds manually we prevent bios of settings ds 0x00 (it might randomly set 0x7c0 or 0x00)
    mov es, ax ; es - extra segment
    mov ss, ax ; set stack segment to be 0
    mov sp, 0x7c00 ; set the stack pointer
    sti ; enable interrupts

.load_protected:
    cli
    lgdt[gdt_descriptor]    ; load gdt
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32
    jmp $

;Global Descriptor Table (GDT)
gdt_start:
gdt_null:
     dd 0x0 ; 32 bits of 0
     dd 0x0 ; 32 bits of 0

; offset 0x8. These are just some default values
gdt_code:       ; CS (code segment) should point to this
    dw 0xffff   ; segment limit first 0-15 bits
    dw 0        ; base first 0-15 bits
    db 0        ; base 16-23 bits
    db 0x9a;    ; access byte
    db 11001111b; high 5 bit flags and low 4 bit flags
    db 0        ; base 24-31 bits

; offset 0x10
gdt_data:       ; ds, ss, es, fs, gs should be linked with it
    dw 0xffff   ; segment limit first 0-15 bits
    dw 0        ; base first 0-15 bits
    db 0        ; base 16-23 bits
    db 0x92;    ; access byte
    db 11001111b; high 5 bit flags and low 4 bit flags
    db 0        ; base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; gdt size
    dd gdt_start ; gdt offset

[BITS 32]
load32:
    mov eax, 1          ; the starting sector we wanna load from
    mov ecx, 100        ; total number of sectors we wanna load
    mov edi, 0x0100000  ; 1Mb. The address we want to load sectors to
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax    ; backup the LBA
    ; send the highest 8 bits of the LBA to hard disk controller
    shr eax, 24 ; shift eax on 24 bits to the right
    or eax, 0xE0 ; select the master drive
    mov dx, 0x1F6
    out dx, al
    ; finished sending the highest 8 bits of the lba

    ; send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; finished sending the total sectors to read

    ; send more bits of the LBA
    mov eax, ebx ; restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; finished sending more bits of the LBA

    ; send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx    ; restore the backup LBA
    shr eax, 8
    out dx, al
    ; finished sending more bits of the LBA

    ; send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx; restore the backup LBA
    shr eax, 16
    out dx, al
    ; finished sending upper 16 bits of the LBA

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ; read all sectors into memory
.next_sector:
    push ecx

    ; check if we need to read
.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

    ; we need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; end of reading sectors into memory
    ret

times 510-($ - $$) db 0 ; it means that we need to fill at least 510 bytes of data
dw 0xAA55   ; 511th and 512th bytes have to be 0x55 and 0xAA, as a flag that everything goes ok. 0xAA55 will be flipped to 0x55AA