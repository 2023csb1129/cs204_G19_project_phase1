
.text
// --- R-type instructions (12) ---
add   x1, x2, x3
sub   x4, x5, x6
mul   x7, x8, x9
div   x10, x11, x12
rem   x13, x14, x15
and   x16, x17, x18
or    x19, x20, x21
xor   x22, x23, x24
sll   x25, x26, x27
srl   x28, x29, x30
sra   x2, x3, x4
slt   x3, x4, x5

// --- I-type instructions (8) ---
addi  x6, x7, 10
andi  x8, x9, 20
ori   x10, x11, 30
lb    x12, 0(x13)
lh    x14, 1(x15)
lw    x16, 2(x17)
ld    x18, 3(x19)
jalr  x20, x21, 40

// --- Branch instructions (B-type: 4) ---
// Branch targets are defined later.
beq   x22, x23, label1
bne   x24, x25, label2
bge   x26, x27, label3
blt   x28, x29, label4

// --- Store instructions (S-type: 4) ---
sb    x30, 5(x1)
sh    x2, 6(x3)
sw    x4, 7(x5)
sd    x6, 8(x7)

// --- UJ-type instruction (1) ---
jal   x8, label5

// --- U-type instructions (2) ---
lui   x9, 1000
auipc x10, 2000

// --- Branch Target Labels ---
// These labels are used by branch and jump instructions above.
label1:
    addi x1, x1, 1

label2:
    andi x2, x2, 2

label3:
    ori  x3, x3, 3

label4:
    lb   x4, 0(x5)

label5:
    lh   x6, 1(x7)

.data
// --- Data Segment Examples ---
.byte  0xAA, 0xBB
.half  0x1234, 0x5678
.word  100, 200
.dword 300, 400
.asciz "Hello"








