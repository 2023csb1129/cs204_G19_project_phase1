

/* Implementation file for myRISCVSim */

#include "myRISCVSim.h"
#include <cstdlib>
#include <cstdio>

#define MEM_SIZE 8192
#define DATA_OFFSET 0x10000000

// Memory array
static unsigned char MEM[MEM_SIZE];

// Processor structure grouping registers and state
struct Processor {
    unsigned int PC;            // Program Counter
    unsigned int IR;            // Instruction Register
    unsigned int R[32];         // Register File
    unsigned int operand1;      // Temporary operand
    unsigned int operand2;      // Temporary operand
    unsigned int dest_reg;      // Destination register index
    unsigned int alu_result;    // ALU output
    unsigned int clock;         // Clock cycle counter
    int skip_pc_increment;      // Flag to skip PC update
    int N, C, V, Z;             // Flags (unused)
};

// Global processor state
static Processor cpu;

// Macros to extract instruction fields
#define OPCODE(x)    ((x) & 0x7F)
#define RD(x)        (((x) >> 7) & 0x1F)
#define FUNCT3(x)    (((x) >> 12) & 0x7)
#define RS1(x)       (((x) >> 15) & 0x1F)
#define RS2(x)       (((x) >> 20) & 0x1F)
#define FUNCT7(x)    (((x) >> 25) & 0x7F)

// Runs the simulation loop
void run_RISCVsim() {
  while (1) {
    fetch();    // Fetch instruction
    decode();   // Decode instruction
    execute();  // Execute instruction
    mem();      // Memory operation
    write_back(); // Write results back
    cpu.clock++;
    std::printf("Clock Cycle = %u\n\n", cpu.clock);
  }
}

// Resets processor state and memory
void reset_proc() {
  for (int i = 0; i < 32; i++)
      cpu.R[i] = 0;
  for (int i = 0; i < MEM_SIZE; i++)
      MEM[i] = 0;
  cpu.PC = 0;
  cpu.IR = 0;
  cpu.clock = 0;
  cpu.skip_pc_increment = 0;
}

// Loads the program into memory
void load_program_memory(char *file_name) {
  FILE *fp = std::fopen(file_name, "r");
  if (fp == nullptr) {
    std::printf("Error opening input mem file\n");
    std::exit(1);
  }
  unsigned int address, instruction;
  while (std::fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    write_word(reinterpret_cast<char*>(MEM), address, instruction);
  }
  std::fclose(fp);
}

// Writes data memory to a file
void write_data_memory() {
  FILE *fp = std::fopen("data_out.mem", "w");
  if (fp == nullptr) {
    std::printf("Error opening data_out.mem for writing\n");
    return;
  }
  for (unsigned int addr = DATA_OFFSET; addr < DATA_OFFSET + (MEM_SIZE - 4096); addr += 4) {
    unsigned int value = read_word(reinterpret_cast<char*>(MEM), addr);
    if (value != 0) {
      std::fprintf(fp, "%08x %08x\n", addr, value);
    }
  }
  std::fclose(fp);
}

// Exits the simulator and dumps registers
void swi_exit() {
  write_data_memory();
  std::printf("\n=== REGISTER DUMP ===\n");
  for (int i = 0; i < 32; i++) {
    std::printf("R%-2d = %d\n", i, cpu.R[i]);
  }
  std::printf("\nFinal array:\n");
  for (int i = 0; i < 10; i++) {
    int addr = DATA_OFFSET + i * 4;
    int val = read_word(reinterpret_cast<char*>(MEM), addr);
    std::printf("[%d] = %d\n", i, val);
  }
  std::exit(0);
}

// Fetch stage: get instruction from memory
void fetch() {
  cpu.IR = read_word(reinterpret_cast<char*>(MEM), cpu.PC);
  std::printf("FETCH: Fetch instruction 0x%08X from address 0x%08X\n", cpu.IR, cpu.PC);
}

// Decode stage: interpret instruction fields
void decode() {
  unsigned int opcode = OPCODE(cpu.IR);
  if (cpu.IR == 0xEF000011) {
    std::printf("DECODE: Exit instruction encountered\n");
    swi_exit();
  }
  // R-type, I-type, Load, Store, Branch, JAL, JALR, LUI, AUIPC handled below...
  if (opcode == 0x33) { // R-type
    unsigned int rd = RD(cpu.IR);
    unsigned int funct3 = FUNCT3(cpu.IR);
    unsigned int rs1 = RS1(cpu.IR);
    unsigned int rs2 = RS2(cpu.IR);
    unsigned int funct7 = FUNCT7(cpu.IR);
    cpu.operand1 = cpu.R[rs1];
    cpu.operand2 = cpu.R[rs2];
    cpu.dest_reg = rd;
    if (funct3 == 0x0 && funct7 == 0x00) {
      std::printf("DECODE: Operation is ADD, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x0 && funct7 == 0x20) {
      std::printf("DECODE: Operation is SUB, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x7 && funct7 == 0x00) {
      std::printf("DECODE: Operation is AND, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x6 && funct7 == 0x00) {
      std::printf("DECODE: Operation is OR, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x1 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SLL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x5 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SRL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x4 && funct7 == 0x00) {
      std::printf("DECODE: Operation is XOR, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x2 && funct7 == 0x00) {
      std::printf("DECODE: Operation is SLT, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x5 && funct7 == 0x20) {
      std::printf("DECODE: Operation is SRA, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x0 && funct7 == 0x01) {
      std::printf("DECODE: Operation is MUL, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x4 && funct7 == 0x01) {
      std::printf("DECODE: Operation is DIV, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x6 && funct7 == 0x01) {
      std::printf("DECODE: Operation is REM, operands R%d and R%d, destination R%d\n", rs1, rs2, rd);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
  }
  else if (opcode == 0x13) {  // I-type instructions
    unsigned int rd = RD(cpu.IR);
    unsigned int funct3 = FUNCT3(cpu.IR);
    unsigned int rs1 = RS1(cpu.IR);
    if (funct3 == 0x1 || funct3 == 0x5) {
      int shamt = (cpu.IR >> 20) & 0x1F;
      cpu.operand1 = cpu.R[rs1];
      cpu.operand2 = shamt;
      cpu.dest_reg = rd;
      if (funct3 == 0x1) {
         std::printf("DECODE: Operation is SLLI, source R%d, shamt %d, dest R%d\n", rs1, shamt, rd);
      } else {
         unsigned int funct7 = (cpu.IR >> 25) & 0x7F;
         if (funct7 == 0x00) {
           std::printf("DECODE: Operation is SRLI, source R%d, shamt %d, dest R%d\n", rs1, shamt, rd);
         } else if (funct7 == 0x20) {
           std::printf("DECODE: Operation is SRAI, source R%d, shamt %d, dest R%d\n", rs1, shamt, rd);
         }
      }
    } else {
       int imm = (cpu.IR >> 20) & 0xFFF;
       if (imm & 0x800) imm |= 0xFFFFF000;
       cpu.operand1 = cpu.R[rs1];
       cpu.operand2 = imm;
       cpu.dest_reg = rd;
       if (funct3 == 0x0) {
         std::printf("DECODE: Operation is ADDI, R%d + %d -> R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read R%d = %d\n", rs1, cpu.operand1);
       }
       else if (funct3 == 0x2) {
         std::printf("DECODE: Operation is SLTI, compare R%d < %d, dest R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read R%d = %d\n", rs1, cpu.operand1);
       }
       else if (funct3 == 0x7) {
         std::printf("DECODE: Operation is ANDI, R%d & %d -> R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read R%d = %d\n", rs1, cpu.operand1);
       }
       else if (funct3 == 0x6) {
         std::printf("DECODE: Operation is ORI, R%d | %d -> R%d\n", rs1, imm, rd);
         std::printf("DECODE: Read R%d = %d\n", rs1, cpu.operand1);
       }
    }
  }
  else if (opcode == 0x03) {  // Load instructions
    unsigned int rd = RD(cpu.IR);
    unsigned int funct3 = FUNCT3(cpu.IR);
    unsigned int rs1 = RS1(cpu.IR);
    int imm = (int)(cpu.IR >> 20);
    cpu.operand1 = cpu.R[rs1];
    cpu.operand2 = imm;
    cpu.dest_reg = rd;
    if (funct3 == 0x2) {
      std::printf("DECODE: Operation is LW, base R%d, offset %d, dest R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base R%d = %d\n", rs1, cpu.operand1);
    }
    else if (funct3 == 0x0) {
      std::printf("DECODE: Operation is LB, base R%d, offset %d, dest R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base R%d = %d\n", rs1, cpu.operand1);
    }
    else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is LH, base R%d, offset %d, dest R%d\n", rs1, imm, rd);
      std::printf("DECODE: Read base R%d = %d\n", rs1, cpu.operand1);
    }
  }
  else if (opcode == 0x23) {  // Store instructions
    unsigned int imm4_0  = (cpu.IR >> 7) & 0x1F;
    unsigned int funct3  = (cpu.IR >> 12) & 0x07;
    unsigned int rs1     = (cpu.IR >> 15) & 0x1F;
    unsigned int rs2     = (cpu.IR >> 20) & 0x1F;
    unsigned int imm11_5 = (cpu.IR >> 25) & 0x7F;
    int imm = ((imm11_5 << 5) | imm4_0);
    if (imm & 0x800) imm |= 0xFFFFF000;
    cpu.operand1 = cpu.R[rs1];
    cpu.operand2 = cpu.R[rs2];
    cpu.alu_result = imm;
    cpu.dest_reg = 0;
    if (funct3 == 0x2) {
      std::printf("DECODE: Operation is SW, base R%d, source R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x0) {
      std::printf("DECODE: Operation is SB, base R%d, source R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is SH, base R%d, source R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
  }
  else if (opcode == 0x63) {  // Branch instructions
    unsigned int funct3 = FUNCT3(cpu.IR);
    unsigned int rs1 = RS1(cpu.IR);
    unsigned int rs2 = RS2(cpu.IR);
    int imm = ((cpu.IR >> 31) & 0x1) << 12 |
              ((cpu.IR >> 25) & 0x3F) << 5 |
              ((cpu.IR >> 8) & 0xF) << 1 |
              ((cpu.IR >> 7) & 0x1) << 11;
    if (imm & 0x1000) imm |= 0xFFFFE000;
    cpu.operand1 = cpu.R[rs1];
    cpu.operand2 = cpu.R[rs2];
    cpu.alu_result = imm;
    cpu.dest_reg = 0;
    if (funct3 == 0x0) {
      std::printf("DECODE: Operation is BEQ, compare R%d and R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    } else if (funct3 == 0x1) {
      std::printf("DECODE: Operation is BNE, compare R%d and R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x4) {
      std::printf("DECODE: Operation is BLT, compare R%d < R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
    else if (funct3 == 0x5) {
      std::printf("DECODE: Operation is BGE, compare R%d >= R%d, offset %d\n", rs1, rs2, imm);
      std::printf("DECODE: Read R%d = %d, R%d = %d\n", rs1, cpu.operand1, rs2, cpu.operand2);
    }
  }
  else if (opcode == 0x6F) {  // JAL
    unsigned int rd = RD(cpu.IR);
    int imm = ((cpu.IR >> 31) & 0x1) << 20 |
              ((cpu.IR >> 21) & 0x3FF) << 1 |
              ((cpu.IR >> 20) & 0x1) << 11 |
              ((cpu.IR >> 12) & 0xFF) << 12;
    if (imm & (1 << 20)) imm |= 0xFFF00000;
    cpu.alu_result = imm;
    cpu.dest_reg = rd;
    cpu.operand1 = cpu.PC;
    std::printf("DECODE: Operation is JAL, dest R%d, offset %d\n", rd, imm);
  }
  else if (opcode == 0x67) {  // JALR
    unsigned int rd = RD(cpu.IR);
    unsigned int rs1 = RS1(cpu.IR);
    int imm = (int)(cpu.IR >> 20);
    cpu.operand1 = cpu.R[rs1];
    cpu.operand2 = imm;
    cpu.dest_reg = rd;
    std::printf("DECODE: Operation is JALR, dest R%d, base R%d, offset %d\n", rd, rs1, imm);
    std::printf("DECODE: Read R%d = %d\n", rs1, cpu.operand1);
  }
  else if (opcode == 0x37) {  // LUI
    unsigned int rd = RD(cpu.IR);
    int imm = (int)(cpu.IR & 0xFFFFF000);
    cpu.alu_result = imm;
    cpu.dest_reg = rd;
    std::printf("DECODE: Operation is LUI, immediate %d, dest R%d\n", imm, rd);
  }
  else if (opcode == 0x17) {  // AUIPC
    unsigned int rd = RD(cpu.IR);
    int imm = (int)(cpu.IR & 0xFFFFF000);
    cpu.alu_result = cpu.PC + imm;
    cpu.dest_reg = rd;
    std::printf("DECODE: Operation is AUIPC, PC %d + imm %d -> R%d\n", cpu.PC, imm, rd);
  }
}

// Execute stage: perform ALU operations
void execute() {
  unsigned int opcode = OPCODE(cpu.IR);
  unsigned int funct3 = FUNCT3(cpu.IR);
  unsigned int funct7 = FUNCT7(cpu.IR);
  if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 + cpu.operand2;
    std::printf("EXECUTE: ADD %d + %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x20) {
    cpu.alu_result = cpu.operand1 - cpu.operand2;
    std::printf("EXECUTE: SUB %d - %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x13 && FUNCT3(cpu.IR) == 0x0) {
    cpu.alu_result = cpu.operand1 + cpu.operand2;
    std::printf("EXECUTE: ADDI %d + %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x13 && (FUNCT3(cpu.IR) == 0x1 || FUNCT3(cpu.IR) == 0x5)) {
    if (FUNCT3(cpu.IR) == 0x1) {
      cpu.alu_result = cpu.operand1 << (cpu.operand2 & 0x1F);
      std::printf("EXECUTE: SLLI %d << %d = %d\n", cpu.operand1, cpu.operand2 & 0x1F, cpu.alu_result);
    } else if (FUNCT3(cpu.IR) == 0x5) {
      if (funct7 == 0x00) {
        cpu.alu_result = cpu.operand1 >> (cpu.operand2 & 0x1F);
        std::printf("EXECUTE: SRLI %d >> %d = %d\n", cpu.operand1, cpu.operand2 & 0x1F, cpu.alu_result);
      } else if (funct7 == 0x20) {
        cpu.alu_result = ((int)cpu.operand1) >> (cpu.operand2 & 0x1F);
        std::printf("EXECUTE: SRAI %d >> %d = %d\n", cpu.operand1, cpu.operand2 & 0x1F, cpu.alu_result);
      }
    }
  }
  else if (opcode == 0x03 && funct3 == 0x2) {
    cpu.alu_result = cpu.operand1 + cpu.operand2;
    std::printf("EXECUTE: LW address = %d\n", cpu.alu_result);
  }
  else if (opcode == 0x23 && FUNCT3(cpu.IR) == 0x2) {
    cpu.alu_result = cpu.operand1 + cpu.alu_result;
    std::printf("EXECUTE: SW address = %d\n", cpu.alu_result);
  }
  else if (opcode == 0x63 && funct3 == 0x0) {
    if (cpu.operand1 == cpu.operand2) {
        std::printf("EXECUTE: BEQ taken, PC += %d\n", cpu.alu_result);
        cpu.PC = cpu.PC + cpu.alu_result;
        cpu.skip_pc_increment = 1;
    } else {
        std::printf("EXECUTE: BEQ not taken\n");
    }
  }
  else if (opcode == 0x63 && funct3 == 0x1) {
    if (cpu.operand1 != cpu.operand2) {
        std::printf("EXECUTE: BNE taken, PC += %d\n", cpu.alu_result);
        cpu.PC += cpu.alu_result;
        cpu.skip_pc_increment = 1;
    } else {
        std::printf("EXECUTE: BNE not taken\n");
    }
  }
  else if (opcode == 0x33 && funct3 == 0x7 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 & cpu.operand2;
    std::printf("EXECUTE: AND %d & %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x6F) {
    if (cpu.dest_reg != 0) {
      cpu.R[cpu.dest_reg] = cpu.operand1 + 4;
      std::printf("EXECUTE: JAL store return addr %d in R%d\n", cpu.operand1 + 4, cpu.dest_reg);
    }
    cpu.PC += cpu.alu_result;
    std::printf("EXECUTE: JAL jump to PC = %d\n", cpu.PC);
    cpu.skip_pc_increment = 1;
  }
  else if (opcode == 0x33 && funct3 == 0x6 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 | cpu.operand2;
    std::printf("EXECUTE: OR %d | %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x1 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 << (cpu.operand2 & 0x1F);
    std::printf("EXECUTE: SLL %d << %d = %d\n", cpu.operand1, cpu.operand2 & 0x1F, cpu.alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x5 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 >> (cpu.operand2 & 0x1F);
    std::printf("EXECUTE: SRL %d >> %d = %d\n", cpu.operand1, cpu.operand2 & 0x1F, cpu.alu_result);
  }
  else if (opcode == 0x33 && funct3 == 0x4 && funct7 == 0x00) {
    cpu.alu_result = cpu.operand1 ^ cpu.operand2;
    std::printf("EXECUTE: XOR %d ^ %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x67) {
    if (cpu.dest_reg != 0) {
      cpu.R[cpu.dest_reg] = cpu.PC + 4;
      std::printf("EXECUTE: JALR store return addr %d in R%d\n", cpu.PC + 4, cpu.dest_reg);
    }
    unsigned int target = (cpu.operand1 + cpu.operand2) & ~1;
    std::printf("EXECUTE: JALR jump to addr %d\n", target);
    cpu.PC = target;
    cpu.skip_pc_increment = 1;
  }
  else if (opcode == 0x13 && FUNCT3(cpu.IR) == 0x2) {
    cpu.alu_result = ((int)cpu.operand1 < (int)cpu.operand2) ? 1 : 0;
    std::printf("EXECUTE: SLTI %d < %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x33 && FUNCT3(cpu.IR) == 0x2 && funct7 == 0x00) {
    cpu.alu_result = ((int)cpu.operand1 < (int)cpu.operand2) ? 1 : 0;
    std::printf("EXECUTE: SLT %d < %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x13 && FUNCT3(cpu.IR) == 0x7) {
    cpu.alu_result = cpu.operand1 & cpu.operand2;
    std::printf("EXECUTE: ANDI %d & %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x13 && FUNCT3(cpu.IR) == 0x6) {
    cpu.alu_result = cpu.operand1 | cpu.operand2;
    std::printf("EXECUTE: ORI %d | %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x33 && FUNCT3(cpu.IR) == 0x0 && funct7 == 0x01) {
    cpu.alu_result = cpu.operand1 * cpu.operand2;
    std::printf("EXECUTE: MUL %d * %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x33 && FUNCT3(cpu.IR) == 0x4 && funct7 == 0x01) {
    if (cpu.operand2 != 0) {
      cpu.alu_result = (int)cpu.operand1 / (int)cpu.operand2;
      std::printf("EXECUTE: DIV %d / %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
    } else {
      cpu.alu_result = 0;
      std::printf("EXECUTE: DIV by zero, result set to 0\n");
    }
  }
  else if (opcode == 0x33 && FUNCT3(cpu.IR) == 0x6 && funct7 == 0x01) {
    if (cpu.operand2 != 0) {
      cpu.alu_result = (int)cpu.operand1 % (int)cpu.operand2;
      std::printf("EXECUTE: REM %d %% %d = %d\n", cpu.operand1, cpu.operand2, cpu.alu_result);
    } else {
      cpu.alu_result = 0;
      std::printf("EXECUTE: REM by zero, result set to 0\n");
    }
  }
}

// Memory stage: load/store operations
void mem() {
  unsigned int opcode = OPCODE(cpu.IR);
  unsigned int funct3 = FUNCT3(cpu.IR);
  if (opcode == 0x33) {
    std::printf("MEMORY: No memory operation\n");
  }
  else if (opcode == 0x03 && FUNCT3(cpu.IR) == 0x2) {
    cpu.alu_result = read_word(reinterpret_cast<char*>(MEM), cpu.alu_result);
    std::printf("MEMORY: Load value %d from addr %d\n", cpu.alu_result, cpu.operand1 + cpu.operand2);
  }
  else if (opcode == 0x23 && FUNCT3(cpu.IR) == 0x2) {
    write_word(reinterpret_cast<char*>(MEM), cpu.alu_result, cpu.operand2);
    std::printf("MEMORY: Stored value %d at addr %d\n", cpu.operand2, cpu.alu_result);
  }
  else if (opcode == 0x03 && funct3 == 0x0) {
    cpu.alu_result = MEM[cpu.operand1 + cpu.operand2];
    if (cpu.alu_result & 0x80) cpu.alu_result |= 0xFFFFFF00;
    std::printf("MEMORY: LB value %d from addr %d\n", cpu.alu_result, cpu.operand1 + cpu.operand2);
  }
  else if (opcode == 0x03 && funct3 == 0x1) {
    cpu.alu_result = *(short*)(MEM + cpu.operand1 + cpu.operand2);
    std::printf("MEMORY: LH value %d from addr %d\n", cpu.alu_result, cpu.operand1 + cpu.operand2);
  }
  else if (opcode == 0x23 && funct3 == 0x0) {
    MEM[cpu.alu_result] = cpu.operand2 & 0xFF;
    std::printf("MEMORY: SB value %d at addr %d\n", cpu.operand2 & 0xFF, cpu.alu_result);
  }
  else if (opcode == 0x23 && funct3 == 0x1) {
    *(short*)(MEM + cpu.alu_result) = cpu.operand2 & 0xFFFF;
    std::printf("MEMORY: SH value %d at addr %d\n", cpu.operand2 & 0xFFFF, cpu.alu_result);
  }
}

// Write-back stage: update register file
void write_back() {
  unsigned int opcode = OPCODE(cpu.IR);
  if (opcode == 0x33 || opcode == 0x13 || opcode == 0x03 || opcode == 0x17 || opcode == 0x37) {
    cpu.R[cpu.dest_reg] = cpu.alu_result;
    std::printf("WRITEBACK: Write %d to R%d\n", cpu.alu_result, cpu.dest_reg);
  }
  if (!cpu.skip_pc_increment) {
    cpu.PC += 4;
  }
  cpu.skip_pc_increment = 0;
  std::printf("WRITEBACK: PC = 0x%08X\n", cpu.PC);
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
