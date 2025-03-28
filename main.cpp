
/* main.cpp 
   Purpose of this file: The file handles the input and output, and
   invokes the simulator
*/

#include "myRISCVSim.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("Incorrect number of arguments. Please invoke the simulator as:\n\t./myRISCVSim <input mc file>\n");
        std::exit(1);
    }
  
    // Reset the processor state
    reset_proc();
  
    // Load the program from the .mc file
    load_program_memory(argv[1]);
  
    // Run the simulator
    run_RISCVsim();
  
    return 0;
}
