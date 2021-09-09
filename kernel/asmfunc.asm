
global SetStack
SetStack:
        mov rsp, rdi + 1024 * 1024
        ret

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
	mov ds, 0
	mov es, 0
	mov fs, 0
	mov gs, 0
	push rbp
	mov rbp, rsp
	mov ss, si
	mov rax .next
	push rdi
	push rax
	o64 retf
.next:
	mov rsp, rbp
	pop rbp
	ret
	
~            
global SetCR3
SetCR3:
	mov cr3, rdi
	ret

