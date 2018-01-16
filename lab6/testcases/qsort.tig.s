.text
.global tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp, %ebp
subl $12, %esp
movl %ebx, -12(%ebp)

L35:
movl $16, %eax
movl $-4, %ebx
movl %ebp, %ecx
addl %ebx, %ecx
movl %eax, (%ecx)
movl $-8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl $-4, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $0, %ecx
pushl %ecx
pushl %eax
call initArray
movl %eax, (%ebx)
pushl %ebp
call L3
jmp L34
L34:
movl -12(%ebp), %ebx


leave
ret
.text
.global L3
.type L3, @function
L3:
pushl %ebp
movl %esp, %ebp
subl $8, %esp
movl %ebx, -4(%ebp)

movl %edi, -8(%ebp)

L37:
movl $8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
call L1
movl $8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
movl $-4, %ebx
addl %ebx, %eax
movl (%eax), %eax
movl $1, %ebx
subl %ebx, %eax
pushl %eax
movl $0, %eax
pushl %eax
movl $8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
call L2
movl $0, %ebx
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $-4, %ecx
addl %ecx, %eax
movl (%eax), %edi
movl $1, %eax
subl %eax, %edi
cmp %edi, %ebx
jg L29
L31:
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $-8, %ecx
addl %ecx, %eax
movl (%eax), %ecx
movl $4, %edx
movl %ebx, %eax
imul %edx, %eax
addl %eax, %ecx
movl (%ecx), %eax
pushl %eax
call printi
movl $L30, %eax
pushl %eax
call print
cmp %edi, %ebx
je L29
L32:
movl $1, %eax
addl %eax, %ebx
jmp L31
L29:
movl $L33, %eax
pushl %eax
call print
jmp L36
L36:
movl -4(%ebp), %ebx

movl -8(%ebp), %edi


leave
ret
.section .rodata
L33:
.int 1
.string "\n"
.section .rodata
L30:
.int 1
.string " "
.text
.global L2
.type L2, @function
L2:
pushl %ebp
movl %esp, %ebp
subl $16, %esp
movl %ebx, -8(%ebp)

movl %esi, -12(%ebp)

movl %edi, -4(%ebp)

L39:
movl $12, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
movl %eax, -16(%ebp)

movl $16, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
movl $8, %ebx
movl %ebp, %ecx
addl %ebx, %ecx
movl (%ecx), %ebx
movl $-8, %ecx
addl %ecx, %ebx
movl (%ebx), %ecx
movl $12, %ebx
movl %ebp, %edx
addl %ebx, %edx
movl (%edx), %ebx
movl $4, %edx
imul %edx, %ebx
addl %ebx, %ecx
movl (%ecx), %ebx
movl $12, %ecx
movl %ebp, %edx
addl %ecx, %edx
movl (%edx), %ecx
movl $16, %edx
movl %ebp, %esi
addl %edx, %esi
movl (%esi), %edx
cmp %edx, %ecx
jl L27
L28:
movl $0, %eax
jmp L38
L27:
L25:
movl -16(%ebp), %ecx

cmp %eax, %ecx
jl L26
L8:
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $-8, %ecx
addl %ecx, %eax
movl (%eax), %ecx
movl $4, %edx
movl -16(%ebp), %eax

imul %edx, %eax
addl %eax, %ecx
movl %ebx, (%ecx)
movl $1, %ebx
movl -16(%ebp), %eax

subl %ebx, %eax
pushl %eax
movl $12, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
movl $8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
call L2
movl $16, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
movl $1, %ebx
movl -16(%ebp), %eax

addl %ebx, %eax
pushl %eax
movl $8, %eax
movl %ebp, %ebx
addl %eax, %ebx
movl (%ebx), %eax
pushl %eax
call L2
jmp L28
L26:
L15:
movl -16(%ebp), %ecx

cmp %eax, %ecx
jl L10
L11:
movl $0, %ecx
L12:
movl $0, %edx
cmp %edx, %ecx
jne L16
L9:
movl $8, %ecx
movl %ebp, %edx
addl %ecx, %edx
movl (%edx), %ecx
movl $-8, %edx
addl %edx, %ecx
movl (%ecx), %edx
movl $4, %esi
movl %eax, %ecx
imul %esi, %ecx
addl %ecx, %edx
movl (%edx), %edx
movl $8, %ecx
movl %ebp, %esi
addl %ecx, %esi
movl (%esi), %ecx
movl $-8, %esi
addl %esi, %ecx
movl (%ecx), %ecx
movl $4, %esi
movl -16(%ebp), %edi

imul %esi, %edi
addl %edi, %ecx
movl %edx, (%ecx)
L23:
movl -16(%ebp), %ecx

cmp %eax, %ecx
jl L18
L19:
movl $0, %ecx
L20:
movl $0, %edx
cmp %edx, %ecx
jne L24
L17:
movl $8, %ecx
movl %ebp, %edx
addl %ecx, %edx
movl (%edx), %ecx
movl $-8, %edx
addl %edx, %ecx
movl (%ecx), %edx
movl $4, %esi
movl -16(%ebp), %ecx

imul %esi, %ecx
addl %ecx, %edx
movl (%edx), %edx
movl $8, %ecx
movl %ebp, %esi
addl %ecx, %esi
movl (%esi), %ecx
movl $-8, %esi
addl %esi, %ecx
movl (%ecx), %esi
movl $4, %edi
movl %eax, %ecx
imul %edi, %ecx
addl %ecx, %esi
movl %edx, (%esi)
jmp L25
L10:
movl $1, %ecx
movl $8, %edx
movl %ebp, %esi
addl %edx, %esi
movl (%esi), %edx
movl $-8, %esi
addl %esi, %edx
movl (%edx), %esi
movl $4, %edi
movl %eax, %edx
imul %edi, %edx
addl %edx, %esi
movl (%esi), %edx
cmp %edx, %ebx
jle L13
L14:
movl $0, %ecx
L13:
jmp L12
L16:
movl $1, %ecx
subl %ecx, %eax
jmp L15
L18:
movl $1, %ecx
movl $8, %edx
movl %ebp, %esi
addl %edx, %esi
movl (%esi), %edx
movl $-8, %esi
addl %esi, %edx
movl (%edx), %edx
movl $4, %esi
movl -16(%ebp), %edi

imul %esi, %edi
addl %edi, %edx
movl (%edx), %edx
cmp %edx, %ebx
jge L21
L22:
movl $0, %ecx
L21:
jmp L20
L24:
movl $1, %edx
movl -16(%ebp), %ecx

addl %edx, %ecx
movl %ecx, -16(%ebp)

jmp L23
L38:
movl -8(%ebp), %ebx

movl -12(%ebp), %esi

movl -4(%ebp), %edi


leave
ret
.text
.global L1
.type L1, @function
L1:
pushl %ebp
movl %esp, %ebp
subl $12, %esp
movl %ebx, -4(%ebp)

movl %esi, -8(%ebp)

movl %edi, -12(%ebp)

L41:
movl $0, %ebx
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $-4, %ecx
addl %ecx, %eax
movl (%eax), %esi
movl $1, %eax
subl %eax, %esi
cmp %esi, %ebx
jg L5
L6:
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $-4, %ecx
addl %ecx, %eax
movl (%eax), %eax
subl %ebx, %eax
movl $8, %ecx
movl %ebp, %edx
addl %ecx, %edx
movl (%edx), %ecx
movl $-8, %edx
addl %edx, %ecx
movl (%ecx), %edx
movl $4, %edi
movl %ebx, %ecx
imul %edi, %ecx
addl %ecx, %edx
movl %eax, (%edx)
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
pushl %eax
call L0
cmp %esi, %ebx
je L5
L7:
movl $1, %eax
addl %eax, %ebx
jmp L6
L5:
movl $0, %eax
jmp L40
L40:
movl -4(%ebp), %ebx

movl -8(%ebp), %esi

movl -12(%ebp), %edi


leave
ret
.text
.global L0
.type L0, @function
L0:
pushl %ebp
movl %esp, %ebp
subl $0, %esp
L43:
movl $L4, %eax
pushl %eax
call print
jmp L42
L42:

leave
ret
.section .rodata
L4:
.int 0
.string ""
