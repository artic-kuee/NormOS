; asmfunc.asm
;
; System V AMD64 Calling Convention
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

extern kernel_main_stack
extern KernelMainNewStack
global KernelMain
KernelMain
    mov rsp, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack

global DisableInt
DisableInt:
        cli
        ret

global EnableInt
EnableInt:
        sti
        ret

global LoadIDT
LoadIDT:
	push rbp
	mov rbp, rsp
    sub rsp, 10
    mov [rsp], di
    mov [rsp + 2], esi
    lidt [rsp]
    mov rsp, rbp
    pop rbp
    ret

global LoadGDT
LoadGDT:
    push rbp
	mov rbp, rsp
	sub rsp, 10
	mov [rsp], di
	mov [rsp + 2], esi
	lgdt [rsp]
	mov rsp, rbp
	pop rbp
    ret

global LoadTR
LoadTR:
	ltr di
	ret


global InitSegmentResistors
InitSegmentResistors:
	mov ds, dx
	mov es, dx
	mov fs, dx
	mov gs, dx
	push rbp
	mov rbp, rsp
	mov ss, si
	mov rax, .next
	push rdi
	push rax
	o64 retf
.next:
	mov rsp, rbp
	pop rbp
	ret
	
            
global SetCR3
SetCR3:
	mov cr3, rdi
	ret

