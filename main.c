/* 

The project is developed as part of CS204: Computer Architecture class Project Phase 2 and Phase 3: Functional Simulator for subset of RISCV Processor

Developer's Names:
Group No:
Developer's Email ids:
Date: 

*/

/* main.c 
   Purpose of this file: The file handles the input and output, and
   invokes the simulator
*/

#include "myRISCVSim.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Incorrect number of arguments. Please invoke the simulator as:\n\t./myRISCVSim <input mc file>\n");
        exit(1);
    }
  
    // Reset the processor state
    reset_proc();
  
    // Load the program from the .mc file
    load_program_memory(argv[1]);
  
    // Run the simulator
    run_RISCVsim();
  
    return 0;
}
