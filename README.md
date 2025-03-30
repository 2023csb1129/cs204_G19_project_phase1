# cs204_G19_project_Phase 2 Project 

 1. Introduction

This project focuses on implementing a functional simulator for the 32-bit RISC-V Instruction Set Architecture (ISA). The simulator takes machine code input from a `.mc` file (produced in Phase 1), processes one instruction at a time, and simulates the execution of each instruction following the behavior outlined in the RISC-V specification. The simulator consists of several stages, including Fetch, Decode, Execute, Memory Access, and Register Update/Writeback. 

The goal is to develop a program that simulates the core steps of instruction execution and provides a detailed output regarding the changes in system state (registers, memory, and control values) for each instruction executed.

 2. Project Structure

The core components of the project include:

- Main Program: The program that implements the functional simulator.
- Register File: A structure representing the 32 registers in the RISC-V architecture.
- Program Counter (pc): A register that stores the address of the next instruction to fetch.
- Instruction Register (IR): A register that stores the instruction currently being executed.
- Temporary Registers: Registers such as RM, RY, etc., which are used for intermediate values during the execution of instructions.
- Memory: A simulated block of memory for data storage used by load and store instructions.

The project also includes additional features, such as a clock cycle tracker, the ability to terminate the simulation, and write the memory state back to the `.mc` file when the simulator is finished.

3. Functionality

The simulator emulates the behavior of a RISC-V processor by executing each instruction in five stages:

 Fetch:
- The instruction is fetched from the memory address pointed to by the Program Counter (PC).
- The instruction is stored in the Instruction Register (IR).
- The PC is updated to point to the next instruction (usually incremented by 4).

 Decode:
- The fetched instruction is decoded to identify the opcode and operands.
- The instruction type is determined (e.g., R-type, I-type, S-type, etc.).
- The operands (either registers or immediate values) are extracted from the instruction.

 Execute:
- The arithmetic or logical operation described by the instruction is performed.
- For control instructions (e.g., branches and jumps), the necessary operations are performed to adjust the Program Counter (PC).
- The result of the execution is stored in temporary registers (e.g., RM, RY).

 Memory Access:
- For memory-related instructions (such as load or store), the memory is accessed.
- If the instruction is a load, data is retrieved from memory and stored in a register.
- If the instruction is a store, data is written to the memory.

Write-back:
- The result of the instruction (if applicable) is written back to the register file.
- If the instruction is a branch or jump, the Program Counter (PC) may be updated accordingly.

## Overview
This project is a RISC-V assembler and simulator that converts RISC-V assembly code into machine code and simulates its execution. It includes both a backend for assembling and simulating the machine code and a frontend for interacting with the system via a graphical user interface (GUI).


4. Data Structures

 Register File:
- The register file consists of 32 registers, each holding a 32-bit value.
- The registers support basic read and write operations.

 Program Counter (PC):
- The PC holds the address of the current instruction.
- It starts at `0x0` and is incremented to point to the next instruction after each fetch cycle.

 Instruction Register (IR):
- The IR holds the current instruction being processed by the simulator.

 Temporary Registers (RM, RY, etc.):
- Temporary registers hold intermediate values during instruction execution.

 Memory:
- A block of memory is used to simulate data storage.
- It is used by instructions that load from or store to memory.

 5. Instruction Execution Steps

 Step 1: Fetch
- The instruction is fetched from memory at the address indicated by the PC.
- The fetched instruction is placed into the Instruction Register (IR).
- The PC is updated to point to the next instruction.

 Step 2: Decode
- The instruction is decoded to extract the opcode and operands.
- The type of instruction (R-type, I-type, etc.) is identified.
- The appropriate operands (registers or immediate values) are fetched from the instruction.

Step 3: Execute
- The specified operation is executed. This could involve arithmetic, logical, or control operations.
- The result is stored in a temporary register, such as RM or RY.

 Step 4: Memory Access
- If the instruction involves accessing memory (load or store), the necessary action is performed:
  - For load instructions, data is fetched from memory and placed in the destination register.
  - For store instructions, data from a register is written to memory.

 Step 5: Write-back
- The result of the instruction is written back to the appropriate register in the register file.
- If the instruction involves branching or jumping, the PC may be updated to reflect the new address.

6. Additional Instructions and Features

 Exit Instruction:
- The simulator includes an exit instruction that terminates execution and writes the state of the memory back to the `.mc` file before shutting down.

 Clock Cycle:
- A variable `clock` is used to track the number of clock cycles.
- For each instruction executed, the clock is incremented, and the number of cycles is displayed at the end of each instruction execution cycle.

 Instruction Messages:
- As the simulator processes each instruction, it outputs messages describing the actions being taken and the changes to the internal state (e.g., updated registers, memory, PC).

 7. Testing and Sample Programs

The simulator is tested using machine code for the following programs:

- Fibonacci Program: A program that computes Fibonacci numbers.
- Factorial Program: A program that calculates the factorial of a number.
- Bubble Sort Program: A program that implements the bubble sort algorithm.

These sample programs ensure that the simulator works correctly with various types of RISC-V instructions.

 8. GUI

An graphical user interface (GUI) could be implemented to visually display the execution of instructions. The GUI would show the state of the registers, memory, and Program Counter during each stage of execution, allowing the user to monitor the simulation in real-time. This is an optional feature and can provide bonus points in the project evaluation.

## Requirements
Ensure you have the following installed:
- Node.js and npm
- A C++ compiler (GCC/Clang/MSVC)
- Any required dependencies for the backend (if applicable)
- Nlohmann JSON library


 9. Implementation Plan

The implementation of the simulator involves the following steps:

1. Define Data Structures: Create the structures for the registers, memory, and PC.
2. Implement Instruction Stages: Implement the five stages of instruction execution (Fetch, Decode, Execute, Memory Access, and Write-back).
3. Parse the Input: Implement the logic to parse the `.mc` file and fetch instructions.
4. Simulate Execution: Write the control flow to execute each instruction, processing each through the five stages.
5. Handle Exit Instruction: Implement functionality to exit the simulator and write the memory state back to the `.mc` file.
6. Testing: Run the simulator using sample programs (Fibonacci, Factorial, Bubble Sort) to verify its correctness.

10. Conclusion

The functional simulator for the 32-bit RISC-V ISA provides an insightful tool for understanding how RISC-V instructions are executed in a pipelined processor. The simulator accurately models the five stages of instruction execution and includes additional features such as clock cycle tracking and memory state saving. By running the simulator with real-world programs, users can see how instructions interact with registers, memory, and the Program Counter in a RISC-V system.

11. References

- RISC-V Specification: The official specification for RISC-V, detailing the instruction set and processor architecture.
- RISC-V Simulator Documentation: Open-source RISC-V simulators that helped inform the design of this project.
- C Programming Guide: Documentation and resources for implementing the simulator in C.


