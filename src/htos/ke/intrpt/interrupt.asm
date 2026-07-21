BITS 64

SECTION .text

GLOBAL KxIntHandlerTable

EXTERN DispatchException
EXTERN DispatchHardware
EXTERN Eoi


%MACRO PUSH_ALL 0
	push    rax
	push    rbx
	push    rcx
	push    rdx
	push    rbp
	push    rsi
	push    rdi
	push    r8
	push    r9
	push    r10
	push    r11
	push    r12
	push    r13
	push    r14
	push    r15
	
	mov     rax, ds
    push    rax
    mov     rax, es
    push    rax
    mov     rax, fs
    push    rax
    mov     rax, gs
    push    rax

	mov     rax, cr3
    push    rax

    mov     rax, dr0
    push    rax
    mov     rax, dr1
    push    rax
    mov     rax, dr2
    push    rax
    mov     rax, dr3
    push    rax
    mov     rax, dr6
    push    rax
    mov     rax, dr7
    push    rax

    sub     rsp, 512 + 24
    fxsave64 [rsp]
%ENDMACRO

%MACRO POP_ALL 0
    fxrstor64 [rsp]
    add     rsp, 512 + 24

    pop     rax
    mov     dr7, rax
    pop     rax
    mov     dr6, rax
    pop     rax
    mov     dr3, rax
    pop     rax
    mov     dr2, rax
    pop     rax
    mov     dr1, rax
    pop     rax
    mov     dr0, rax

    pop     rax
    mov     cr3, rax

    pop     rax
    pop     rax

    pop     rax
    mov     es, rax
    pop     rax
    mov     ds, rax

    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rbp
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax
%ENDMACRO

ALIGN 16
KxExceptionInterrupt:
    PUSH_ALL
    mov rdi, rsp
    sub rsp, 0x28
    call DispatchException
    add rsp, 0x28
    POP_ALL
    add rsp, 16
    iretq

ALIGN 16
KxHardwareInterrupt:
    PUSH_ALL
    mov rdi, rsp
    sub rsp, 0x28
    call DispatchHardware
    call Eoi
    add rsp, 0x28
    POP_ALL
    add rsp, 16
    iretq

ALIGN 16
KxSpuriousInterrupt:
    push 0
    push 0xFF
    PUSH_ALL
    mov rcx, rsp
    sub rsp, 0x28
    call Eoi
    add rsp, 0x28
    POP_ALL
    add rsp, 16
    iretq

    
%MACRO REGULAR_EXCEPTION 1
ALIGN 16
global KxExceptionInt%1
KxExceptionInt%1:
    push 0
    push %1
    jmp KxExceptionInterrupt
%ENDMACRO

%MACRO ERROR_EXCEPTION 1
ALIGN 16
global KxExceptionInt%1
KxExceptionInt%1:
    push %1
    jmp KxExceptionInterrupt
%ENDMACRO

%MACRO HARDWARE_INTERRUPT 1
ALIGN 16
global KxHardwareInt%1
KxHardwareInt%1:
    push 0
    push %1
    jmp KxHardwareInterrupt
%ENDMACRO

%define HAS_ERROR_CODE(i) (i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21 || i == 29 || i == 30)

%ASSIGN i 0
%REP 32
%IF !HAS_ERROR_CODE(i)
    REGULAR_EXCEPTION i
%ELSE
    ERROR_EXCEPTION i
%ENDIF
%ASSIGN i i + 1
%ENDREP

HARDWARE_INTERRUPT 33
HARDWARE_INTERRUPT 34
HARDWARE_INTERRUPT 35
HARDWARE_INTERRUPT 36
HARDWARE_INTERRUPT 37
HARDWARE_INTERRUPT 38
HARDWARE_INTERRUPT 39

REGULAR_EXCEPTION 41
REGULAR_EXCEPTION 44

HARDWARE_INTERRUPT 40
HARDWARE_INTERRUPT 42
HARDWARE_INTERRUPT 43

%ASSIGN i 45
%REP 209
HARDWARE_INTERRUPT i
%ASSIGN i i + 1
%ENDREP

ALIGN 16
KxIntHandlerTable:
%ASSIGN     VECTOR 0
%REP        32
dq          KxExceptionInt%+VECTOR
%ASSIGN     VECTOR VECTOR+1
%ENDREP
dq          KxHardwareInt33
dq          KxHardwareInt34
dq          KxHardwareInt35
dq          KxHardwareInt36
dq          KxHardwareInt37
dq          KxHardwareInt38
dq          KxHardwareInt39
dq          KxHardwareInt40
dq          KxExceptionInt41
dq          KxHardwareInt42
dq          KxHardwareInt43
dq          KxExceptionInt44
%ASSIGN     VECTOR 45
%REP        209
dq          KxHardwareInt%+VECTOR
%ASSIGN     VECTOR VECTOR+1
%ENDREP
dq          KxSpuriousInterrupt
