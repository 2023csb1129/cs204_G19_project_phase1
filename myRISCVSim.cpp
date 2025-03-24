/* 

The project is developed as part of CS204: Computer Architecture class Project Phase 2 and Phase 3: Functional Simulator for subset of RISCV Processor

Developer's Names:
Group No:
Developer's Email ids:
Date: 

*/

/* myRISCVSim.cpp
   Purpose of this file: Implementation file for myRISCVSim
*/

#include "myRISCVSim.h"
#include <cstdlib>
#include <cstdio>

// Register file
static unsigned int R[32];

static int skip_pc_increment = 0;

// Memory
#define MEM_SIZE 8192
#define DATA_OFFSET 0x10000000
static unsigned char MEM[MEM_SIZE];

// Flags (not used yet)
static int N, C, V, Z;

// PC, IR, operand registers
static unsigned int PC = 0x0;
static unsigned int IR = 0;
static unsigned int operand1;
static unsigned int operand2;
static unsigned int dest_reg;
static unsigned int alu_result;

// Clock
static unsigned int clock = 0;

#define OPCODE(x)    ((x) & 0x7F)
#define RD(x)        (((x) >> 7) & 0x1F)
#define FUNCT3(x)    (((x) >> 12) & 0x7)
#define RS1(x)       (((x) >> 15) & 0x1F)
#define RS2(x)       (((x) >> 20) & 0x1F)
#define FUNCT7(x)    (((x) >> 25) & 0x7F)

void run_RISCVsim() {
  while (1) {
    fetch();
    decode();
    execute();
    mem();
    write_back();
    clock++;
    std::printf("Clock Cycle = %u\n\n", clock);
  }
}

void reset_proc() {
  // Clear all registers
  for (int i = 0; i < 32; i++)
      R[i] = 0;
  // Clear entire memory
  for (int i = 0; i < MEM_SIZE; i++)
      MEM[i] = 0;
  PC = 0;
  IR = 0;
  clock = 0;
}

void load_program_memory(char *file_name) {
  FILE *fp;
  unsigned int address, instruction;
  fp = std::fopen(file_name, "r");
  if (fp == nullptr) {
    std::printf("Error opening input mem file\n");
    std::exit(1);
  }
  while (std::fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    write_word((char*)MEM, address, instruction);
  }
  std::fclose(fp);
}

void write_data_memory() {
  FILE *fp = std::fopen("data_out.mem", "w");
  if (fp == nullptr) {
    std::printf("Error opening data_out.mem for writing\n");
    return;
  }
  // Data memory is mapped to addresses starting at DATA_OFFSET.
  // It occupies (MEM_SIZE - 4096) bytes.
  for (unsigned int addr = DATA_OFFSET; addr < DATA_OFFSET + (MEM_SIZE - 4096); addr += 4) {
    unsigned int value = read_word((char*)MEM, addr);
    if (value != 0) {
      std::fprintf(fp, "%08x %08x\n", addr, value);
    }
  }
  std::fclose(fp);
}

void swi_exit() {
  write_data_memory();
  
  std::printf("\n=== REGISTER DUMP ===\n");
  for (int i = 0; i < 32; i++) {
    std::printf("R%-2d = %d\n", i, R[i]);
  }
  std::printf("\nFinal array:\n");
  for (int i = 0; i < 10; i++) {
    int addr = DATA_OFFSET + i * 4;
    int val = read_word((char*)MEM, addr);
    std::printf("[%d] = %d\n", i, val);
  }
  std::exit(0);
}

void fetch() {
  IR = read_word((char*)MEM, PC);
  std::printf("FETCH: Fetch instruction 0x%08X from address 0x%08X\n", IR, PC);
}

void decode() {
  unsigned int opcode = OPCODE(IR);
  if (IR == 0xEF000011) {
    std::printf("DECODE: Exit instruction encountered\n");
    swi_exit();
  }

  if (opcode == 0x33) { // R-type instructions
    unsigned int rd = RD(IR);
    unsigned int funct3 = FUNCT3(IR);
    unsigned int rs1 = RS1(IR);
    unsigned int rs2 = RS2(IR);
    unsigned int funct7 = FUNCT7(IR);

    operand1 = R[rs1];
    operand2 = R[rs2];
    dest_reg = rd;

    if (funct3 == 0x0 && funct7 == 0x00) {
      std::printf("DECODE: Operation is ADD, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x0 && funct7 == 0x20) {
      std::printf("DECODE: Operation is SUB, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x7 && funct7 == 0x00) {
      std::printf("DECODE: Operation is AND, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x6 && funct7 == 0x00) {
      std::printf("DECODE: Operation is OR, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x1 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SLL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x5 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SRL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x4 && funct7 == 0x00) {
      std::printf("DECODE: Operation is XOR, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x2 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SLT, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x5 && funct7 == 0x20) {
      std::printf("DECODE: Operation is SRA, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x0 && funct7 == 0x01) {
      std::printf("DECODE: Operation is MUL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x4 && funct7 == 0x01) {
      std::printf("DECODE: Operation is DIV, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x6 && funct7 == 0x01) {
      std::printf("DECODE: Operation is REM, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
  }
  else if (opcode == 0x13) {  // I-type instructions
    unsigned int rd = RD(IR);
    unsigned int funct3 = FUNCT3(IR);
    unsigned int rs1 = RS1(IR);
    // Check if the instruction is a shift (SLLI, SRLI, or SRAI)
    if (funct3 == 0x1 || funct3 == 0x5) {
      int shamt = (IR >> 20) & 0x1F;
      operand1 = R[rs1];
      operand2 = shamt;
      dest_reg = rd;
      if (funct3 == 0x1) {
         // SLLI: the upper 7 bits must be zero
         std::printf("DECODE: Operation is SLLI, source register R%d, shamt %d, destination register R%d\n", rs1, shamt, rd);
      } else { // funct3 == 0x5
         unsigned int funct7 = (IR >> 25) & 0x7F;
         if (funct7 == 0x00) {
           std::printf("DECODE: Operation is SRLI, source register R%d, shamt %d, destination register R%d\n", rs1, shamt, rd);
         } else if (funct7 == 0x20) {
           std::printf("DECODE: Operation is SRAI, source register R%d, shamt %d, destination register R%d\n", rs1, shamt, rd);
         }
      }
    } else {
       int imm = (IR >> 20) & 0xFFF;
       if (imm & 0x800) imm |= 0xFFFFF000;
       operand1 = R[rs1];
       operand2 = imm;
       dest_reg = rd;
       if (funct3 == 0x0) {
         std::printf("DECODE: Operation is ADDI, source register R%d, immediate %d, destination register R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read register R%d = %d\n", rs1, operand1);
       }
       else if (funct3 == 0x2) {
         std::printf("DECODE: Operation is SLTI, compare R%d < %d, destination register R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read register R%d = %d\n", rs1, operand1);
       }
       else if (funct3 == 0x7) {
         std::printf("DECODE: Operation is ANDI, source register R%d, immediate %d, destination register R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read register R%d = %d\n", rs1, operand1);
       }
       else if (funct3 == 0x6) {
         std::printf("DECODE: Operation is ORI, source register R%d, immediate %d, destination register R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read register R%d = %d\n", rs1, operand1);
       }
    }
  }
  else if (opcode == 0x03) {  // Load instructions (I-type)
    unsigned int rd = RD(IR);
    unsigned int funct3 = FUNCT3(IR);
    unsigned int rs1 = RS1(IR);
    int imm = (int)(IR >> 20); // Sign-extended

    operand1 = R[rs1];
    operand2 = imm;
    dest_reg = rd;

    if (funct3 == 0x2) {
      std::printf("DECODE: Operation is LW, base register R%d, offset %d, destination register R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base register R%d = %d\n", rs1, operand1);
    }
    else if (funct3 == 0x0) {
      std::printf("DECODE: Operation is LB, base register R%d, offset %d, destination register R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base register R%d = %d\n", rs1, operand1);
    }
    else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is LH, base register R%d, offset %d, destination register R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base register R%d = %d\n", rs1, operand1);
    }
  }
  else if (opcode == 0x23) {  // S-type (store instructions)
    unsigned int imm4_0  = (IR >> 7) & 0x1F;
    unsigned int funct3  = (IR >> 12) & 0x07;
    unsigned int rs1     = (IR >> 15) & 0x1F;
    unsigned int rs2     = (IR >> 20) & 0x1F;
    unsigned int imm11_5 = (IR >> 25) & 0x7F;

    int imm = ((imm11_5 << 5) | imm4_0);
    if (imm & 0x800) imm |= 0xFFFFF000;

    operand1 = R[rs1];
    operand2 = R[rs2];
    alu_result = imm;
    dest_reg = 0;

    if (funct3 == 0x2) {
      std::printf("DECODE: Operation is SW, base register R%d, source register R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d (base), R%d = %d (value)\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x0) {
      std::printf("DECODE: Operation is SB, base register R%d, source register R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d (base), R%d = %d (value)\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is SH, base register R%d, source register R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d (base), R%d = %d (value)\n", rs1, operand1, rs2, operand2);
    }
  }
  else if (opcode == 0x63) {  // B-type (branch instructions)
    unsigned int funct3  = FUNCT3(IR);
    unsigned int rs1     = RS1(IR);
    unsigned int rs2     = RS2(IR);

    int imm = ((IR >> 31) & 0x1) << 12 |
              ((IR >> 25) & 0x3F) << 5 |
              ((IR >> 8) & 0xF) << 1 |
              ((IR >> 7) & 0x1) << 11;
    if (imm & 0x1000) imm |= 0xFFFFE000;

    operand1 = R[rs1];
    operand2 = R[rs2];
    alu_result = imm;
    dest_reg = 0;

    if (funct3 == 0x0) {
      std::printf("DECODE: Operation is BEQ, compare R%d and R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    } else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is BNE, compare R%d and R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x4) {
      std::printf("DECODE: Operation is BLT, compare R%d < R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
    else if (funct3 == 0x5) {
      std::printf("DECODE: Operation is BGE, compare R%d >= R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, operand1, rs2, operand2);
    }
  }
  else if (opcode == 0x6F) {  // JAL
    unsigned int rd = RD(IR);
    int imm = ((IR >> 31) & 0x1) << 20 |
              ((IR >> 21) & 0x3FF) << 1 |
              ((IR >> 20) & 0x1) << 11 |
              ((IR >> 12) & 0xFF) << 12;
    if (imm & (1 << 20)) imm |= 0xFFF00000;

    alu_result = imm;
    dest_reg = rd;
    operand1 = PC;
    std::printf("DECODE: Operation is JAL, rd = R%d, offset = %d\n", rd, imm);
  }
  else if (opcode == 0x67) {  // JALR
    unsigned int rd = RD(IR);
    unsigned int rs1 = RS1(IR);
    int imm = (int)(IR >> 20);

    operand1 = R[rs1];
    operand2 = imm;
    dest_reg = rd;
    std::printf("DECODE: Operation is JALR, rd = R%d, rs1 = R%d, offset = %d\n", rd, rs1, imm);
    std::printf("DECODE: Read R%d = %d\n", rs1, operand1);
  }
  else if (opcode == 0x37) {  // LUI
    unsigned int rd = RD(IR);
    int imm = (int)(IR & 0xFFFFF000);
    alu_result = imm;
    dest_reg = rd;
    std::printf("DECODE: Operation is LUI, immediate = %d, destination register R%d\n", imm, rd);
  }
  else if (opcode == 0x17) {  // AUIPC
    unsigned int rd = RD(IR);
    int imm = (int)(IR & 0xFFFFF000);
    alu_result = PC + imm;
    dest_reg = rd;
    std::printf("DECODE: Operation is AUIPC, PC = %d, immediate = %d, destination R%d\n", PC, imm, rd);
  }
}

void execute() {
  unsigned int opcode = OPCODE(IR);
  unsigned int funct3 = FUNCT3(IR);
  unsigned int funct7 = FUNCT7(IR);

  if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x00) {
    alu_result = operand1 + operand2;
    std::printf("EXECUTE: ADD %d and %d\n", operand1, operand2);
  }
  else if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x20) {
    alu_result = operand1 - operand2;
    std::printf("EXECUTE: SUB %d and %d\n", operand1, operand2);
  }
  else if (opcode == 0x13 && FUNCT3(IR) == 0x0) {
    alu_result = operand1 + operand2;
    std::printf("EXECUTE: ADDI %d and %d\n", operand1, operand2);
  }
  else if (opcode == 0x13 && (FUNCT3(IR) == 0x1 || FUNCT3(IR) == 0x5)) {
    // Handle shift immediate instructions
    if (FUNCT3(IR) == 0x1) {  // SLLI
      alu_result = operand1 << (operand2 & 0x1F);
      std::printf("EXECUTE: SLLI %d << %d = %d\n", operand1, operand2 & 0x1F, alu_result);
    } else if (FUNCT3(IR) == 0x5) {
      if (funct7 == 0x00) {  // SRLI
        alu_result = operand1 >> (operand2 & 0x1F);
        std::printf("EXECUTE: SRLI %d >> %d = %d\n", operand1, operand2 & 0x1F, alu_result);
      } else if (funct7 == 0x20) {  // SRAI
        alu_result = ((int)operand1) >> (operand2 & 0x1F);
        std::printf("EXECUTE: SRAI %d >> %d = %d\n", operand1, operand2 & 0x1F, alu_result);
      }
    }
  }
  else if (opcode == 0x03 && funct3 == 0x2) {  // LW
    alu_result = operand1 + operand2;
    std::printf("EXECUTE: Compute address %d + %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x23 && FUNCT3(IR) == 0x2) {  // SW
    alu_result = operand1 + alu_result;
    std::printf("EXECUTE: Compute address %d + %d = %d\n", operand1, (int)(alu_result - operand1), alu_result);
  }
  else if (opcode == 0x63 && funct3 == 0x0) {  // BEQ
    if (operand1 == operand2) {
        std::printf("EXECUTE: BEQ taken, PC += %d\n", alu_result);
        PC = PC + alu_result;
        skip_pc_increment = 1;
    } else {
        std::printf("EXECUTE: BEQ not taken\n");
    }
  }
  else if (opcode == 0x63 && funct3 == 0x1) {  // BNE
    if (operand1 != operand2) {
        std::printf("EXECUTE: BNE taken, PC += %d\n", alu_result);
        PC += alu_result;
        skip_pc_increment = 1;
    } else {
        std::printf("EXECUTE: BNE not taken\n");
    }
  }
  else if (opcode == 0x33 && funct3 == 0x7 && funct7 == 0x00) {  // AND
    alu_result = operand1 & operand2;
    std::printf("EXECUTE: AND %d & %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x6F) {  // JAL
    if (dest_reg != 0) {
      R[dest_reg] = operand1 + 4;
      std::printf("EXECUTE: JAL storing return address %d to R%d\n", operand1 + 4, dest_reg);
    }
    PC += alu_result;
    std::printf("EXECUTE: JAL jumping to PC = %d (offset = %d)\n", PC, alu_result);
    skip_pc_increment = 1;
  }
  else if (opcode == 0x33 && funct3 == 0x6 && funct7 == 0x00) {  // OR
    alu_result = operand1 | operand2;
    std::printf("EXECUTE: OR %d | %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x1 && funct7 == 0x00) {  // SLL
    alu_result = operand1 << (operand2 & 0x1F);
    std::printf("EXECUTE: SLL %d << %d = %d\n", operand1, operand2 & 0x1F, alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x5 && funct7 == 0x00) {  // SRL
    alu_result = operand1 >> (operand2 & 0x1F);
    std::printf("EXECUTE: SRL %d >> %d = %d\n", operand1, operand2 & 0x1F, alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x4 && funct7 == 0x00) {  // XOR
    alu_result = operand1 ^ operand2;
    std::printf("EXECUTE: XOR %d ^ %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x67) {  // JALR
    if (dest_reg != 0) {
      R[dest_reg] = PC + 4;
      std::printf("EXECUTE: JALR storing return address %d to R%d\n", PC + 4, dest_reg);
    }
    unsigned int target = (operand1 + operand2) & ~1;
    std::printf("EXECUTE: JALR jumping to address %d\n", target);
    PC = target;
    skip_pc_increment = 1;
  }
  else if (opcode == 0x13 && FUNCT3(IR) == 0x2) {  // SLTI
    alu_result = ((int)operand1 < (int)operand2) ? 1 : 0;
    std::printf("EXECUTE: SLTI %d < %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x33 && FUNCT3(IR) == 0x2 && funct7 == 0x00) {  // SLT
    alu_result = ((int)operand1 < (int)operand2) ? 1 : 0;
    std::printf("EXECUTE: SLT %d < %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x13 && FUNCT3(IR) == 0x7) {  // ANDI
    alu_result = operand1 & operand2;
    std::printf("EXECUTE: ANDI %d & %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x13 && FUNCT3(IR) == 0x6) {  // ORI
    alu_result = operand1 | operand2;
    std::printf("EXECUTE: ORI %d | %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x01) {  // MUL
    alu_result = operand1 * operand2;
    std::printf("EXECUTE: MUL %d * %d = %d\n", operand1, operand2, alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x4 && funct7 == 0x01) {  // DIV
    if (operand2 != 0) {
      alu_result = (int)operand1 / (int)operand2;
      std::printf("EXECUTE: DIV %d / %d = %d\n", operand1, operand2, alu_result);
    } else {
      alu_result = 0;
      std::printf("EXECUTE: DIV by zero, result set to 0\n");
    }
  }
  else if (opcode == 0x33 && funct3 == 0x6 && funct7 == 0x01) {  // REM
    if (operand2 != 0) {
      alu_result = (int)operand1 % (int)operand2;
      std::printf("EXECUTE: REM %d %% %d = %d\n", operand1, operand2, alu_result);
    } else {
      alu_result = 0;
      std::printf("EXECUTE: REM by zero, result set to 0\n");
    }
  }
}

void mem() {
  unsigned int opcode = OPCODE(IR);
  unsigned int funct3 = FUNCT3(IR);
  if (opcode == 0x33) {
    std::printf("MEMORY: No memory operation\n");
  }
  else if (opcode == 0x03 && FUNCT3(IR) == 0x2) {  // LW
    alu_result = read_word((char*)MEM, alu_result);
    std::printf("MEMORY: Load value %d from address %d\n", alu_result, operand1 + operand2);
  }
  else if (opcode == 0x23 && FUNCT3(IR) == 0x2) {  // SW
    write_word((char*)MEM, alu_result, operand2);
    std::printf("MEMORY: Stored value %d at address %d\n", operand2, alu_result);
  }
  else if (opcode == 0x03 && funct3 == 0x0) {  // LB
    alu_result = MEM[operand1 + operand2];
    if (alu_result & 0x80) alu_result |= 0xFFFFFF00;
    std::printf("MEMORY: Load byte (LB) value %d from address %d\n", alu_result, operand1 + operand2);
  }
  else if (opcode == 0x03 && funct3 == 0x1) {  // LH
    alu_result = *(short*)(MEM + operand1 + operand2);
    std::printf("MEMORY: Load halfword (LH) value %d from address %d\n", alu_result, operand1 + operand2);
  }
  else if (opcode == 0x23 && funct3 == 0x0) {  // SB
    MEM[alu_result] = operand2 & 0xFF;
    std::printf("MEMORY: Stored byte (SB) value %d at address %d\n", operand2 & 0xFF, alu_result);
  }
  else if (opcode == 0x23 && funct3 == 0x1) {  // SH
    *(short*)(MEM + alu_result) = operand2 & 0xFFFF;
    std::printf("MEMORY: Stored halfword (SH) value %d at address %d\n", operand2 & 0xFFFF, alu_result);
  }
}

void write_back() {
  unsigned int opcode = OPCODE(IR);
  if (opcode == 0x33 || opcode == 0x13 || opcode == 0x03 || opcode == 0x17 || opcode == 0x37) {
    R[dest_reg] = alu_result;
    std::printf("WRITEBACK: write %d to R%d\n", alu_result, dest_reg);
  }
  if (!skip_pc_increment) {
    PC += 4;
  }
  skip_pc_increment = 0;
  std::printf("WRITEBACK: PC = 0x%08X\n", PC);
}

int read_word(char *mem, unsigned int address) {
  int index = (address >= DATA_OFFSET) ? (address - DATA_OFFSET + 4096) : address;
  int *data = (int*)(mem + index);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int index = (address >= DATA_OFFSET) ? (address - DATA_OFFSET + 4096) : address;
  int *data_p = (int*)(mem + index);
  *data_p = data;
}

/* 
   Note: The main() function has been moved to main.cpp as per the project template.
*/
