EXTERN InstrumentationCallbackHandler : PROC

.CODE

CapturePartialContext MACRO
	pushfq
	;
	; Capture volatile registers
	;
	mov     [rsp+080h], rax
	mov     [rsp+088h], rcx
	mov     [rsp+090h], rdx
	mov     [rsp+0C0h], r8
	mov     [rsp+0C8h], r9
	mov     [rsp+0D0h], r10
	mov     [rsp+0D8h], r11
	movaps  xmmword ptr [rsp+1A8h], xmm0
	movaps  xmmword ptr [rsp+1B8h], xmm1
	movaps  xmmword ptr [rsp+1C8h], xmm2
	movaps  xmmword ptr [rsp+1D8h], xmm3
	movaps  xmmword ptr [rsp+1E8h], xmm4
	movaps  xmmword ptr [rsp+1F8h], xmm5
	;
	; Capture non volatile registers
	;
	mov     [rsp+098h], rbx
	mov     [rsp+0A8h], rbp
	mov     [rsp+0B0h], rsi
	mov     [rsp+0B8h], rdi
	mov     [rsp+0E0h], r12
	mov     [rsp+0E8h], r13
	mov     [rsp+0F0h], r14
	mov     [rsp+0F8h], r15
	;
	; Capture floating point and XMM registers
	;
	fnstcw  word ptr [rsp+108h]
	mov     dword ptr [rsp+10Ah], 0
	movaps  xmmword ptr [rsp+208h], xmm6
	movaps  xmmword ptr [rsp+218h], xmm7
	movaps  xmmword ptr [rsp+228h], xmm8
	movaps  xmmword ptr [rsp+238h], xmm9
	movaps  xmmword ptr [rsp+248h], xmm10
	movaps  xmmword ptr [rsp+258h], xmm11
	movaps  xmmword ptr [rsp+268h], xmm12
	movaps  xmmword ptr [rsp+278h], xmm13
	movaps  xmmword ptr [rsp+288h], xmm14
	movaps  xmmword ptr [rsp+298h], xmm15
	stmxcsr dword ptr [rsp+120h]
	stmxcsr dword ptr [rsp+03Ch]
	;
	; Capture original rsp
	;
	mov     rax, qword ptr gs:[ 2E0h ]
	mov     qword ptr gs:[ 2E0h ], 0
	mov     [rsp+0A0h], rax
	;
	; Capture original pc
	;
	mov     [rsp+100h], r10
	;
	; Capture EFlags register
	;
	mov     eax, [rsp]
	mov     [rsp+04Ch], eax
	;
	; Set ContextFlags to CONTEXT_FULL to exclude the segment selector registers
	;
	mov     dword ptr [rsp+038h], 10000Bh
	add     rsp, 8
ENDM

InstrumentationCallbackEntry PROC
	
	;
	; Temporarily store the unsoiled RSP in TEB->InstrumentationCallbackPreviousSp
	;
	mov qword ptr gs:[2E0h], rsp
	;
	; Align the stack pointer to the lowest 16-byte boundary to adhere to certain functions
	;
	and rsp, 0FFFFFFFFFFFFFFF0h
	;
	; Free up some space and store the processor state here
	;
	sub rsp, 4D0h
	CapturePartialContext
	mov rcx, rsp
	sub rsp, 20h
	call InstrumentationCallbackHandler

InstrumentationCallbackEntry ENDP

END
