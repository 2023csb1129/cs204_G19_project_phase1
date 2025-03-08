
# R-Type Instructions
add x1, x2, x3
sub x4, x5, x6
mul x7, x8, x9
and x10, x11, x12
or x13, x14, x15
xor x16, x17, x18
sll x19, x20, x21
srl x22, x23, x24
sra x25, x26, x27

# I-Type Instructions
addi x5, x6, 10
andi x7, x8, -15
ori x9, x10, 32
lb x11, 20(x12)
lh x13, -12(x14)
lw x15, 128(x16)
jalr x17, 64(x18)

# S-Type (Store) Instructions
sb x19, 8(x20)
sh x21, -16(x22)
sw x23, 40(x24)

# SB-Type (Branch) Instructions
beq x1, x2, 16
bne x3, x4, -8
bge x5, x6, 20
blt x7, x8, -4

# U-Type Instructions
lui x9, 262144  # Large immediate
auipc x10, 8192 # Offset calculation

# UJ-Type (Jump) Instructions
jal x1, 1024
jal x5, -2048
jal x10, 2048
