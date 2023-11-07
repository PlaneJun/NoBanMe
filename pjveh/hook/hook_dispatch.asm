
.code
hookDispatch PROC
lea rsp,[rsp+8h]
push rsp
push rax
push rbx
push rcx
push rdx
push rbp
push rsi
push rdi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
pushfq
sub rsp,40h
movups [rsp+30],xmm0
movups [rsp+20],xmm1
movups [rsp+10],xmm2
movups [rsp],xmm3

;8 byte aglim check
lea rcx,[rsp]
test rsp,8
je s1
mov rax,123456789ABCDEFh
push rax

s1:
sub rsp,20h
mov rax,1212121212121212h
call rax
add rsp,20h
mov rax,123456789ABCDEFh
cmp [rsp],rax
jne s2
pop rax

s2:
movups xmm3,[rsp + 0]
movups xmm2,[rsp + 10h]
movups xmm1,[rsp + 20h]
movups xmm0,[rsp + 30h]
add rsp,40h
popfq
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rdi
pop rsi
pop rbp
pop rdx
pop rcx
pop rbx
pop rax
pop rsp
ret
hookDispatch ENDP
END