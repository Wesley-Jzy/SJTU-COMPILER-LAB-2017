.text
.global tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp, %ebp
subl $20, %esp
L8:
movl $-4, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl %ecx, %eax
movl %eax, -16(%ebp)

movl $4, %eax
pushl %eax
call allocRecord
movl %eax, %eax
movl %eax, %eax
movl %eax, -20(%ebp)

movl $5, %eax
movl $0, %ecx
movl -20(%ebp), %edx

movl %edx, %edx
addl %ecx, %edx
movl %eax, (%edx)
movl -16(%ebp), %eax

movl -20(%ebp), %ecx

movl %ecx, (%eax)
movl $-8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl %ecx, %eax
movl %eax, -12(%ebp)

call getchar
movl %eax, %eax
movl %eax, %eax
movl -12(%ebp), %ecx

movl %eax, (%ecx)
movl $-4, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
pushl %eax
pushl %ebp
call L0
movl %eax, %eax
movl %eax, %eax
pushl %eax
call printi
movl %eax, %eax
movl %eax, %eax
jmp L7
L7:


leave
ret
.text
.global L0
.type L0, @function
L0:
pushl %ebp
movl %esp, %ebp
subl $4, %esp
L10:
movl $0, %eax
movl $-4, %ecx
movl %ebp, %edx
addl %ecx, %edx
movl %eax, (%edx)
pushl %ebp
call L1
movl %eax, %eax
movl $-4, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl %eax, %eax
jmp L9
L9:


leave
ret
.text
.global L1
.type L1, @function
L1:
pushl %ebp
movl %esp, %ebp
subl $4, %esp
L12:
movl $L2, %eax
pushl %eax
call print
movl %eax, %eax
L5:
movl $L4, %eax
pushl %eax
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $8, %ecx
movl %eax, %eax
addl %ecx, %eax
movl (%eax), %eax
movl $-8, %ecx
movl %eax, %eax
addl %ecx, %eax
movl (%eax), %eax
pushl %eax
call stringEqual
movl %eax, %eax
movl %eax, %eax
movl $0, %ecx
cmp %ecx, %eax
je L6
L3:
movl $0, %eax
movl %eax, %eax
jmp L11
L6:
movl $8, %eax
movl %ebp, %ecx
addl %eax, %ecx
movl (%ecx), %eax
movl $8, %ecx
movl %eax, %eax
addl %ecx, %eax
movl (%eax), %eax
movl $-8, %ecx
movl %eax, %eax
addl %ecx, %eax
movl %eax, %eax
movl %eax, -4(%ebp)

call getchar
movl %eax, %ecx
movl %ecx, %ecx
movl -4(%ebp), %edx

movl %ecx, (%edx)
jmp L5
L11:


leave
ret
.section .rodata
L4:
.int 1
.string "h"
.section .rodata
L2:
.int 3
.string "ha\n"
