#ifndef MYRISCVSIM_H
#define MYRISCVSIM_H

void run_RISCVsim();
void reset_proc();
void load_program_memory(char *file_name);
void write_data_memory();
void swi_exit();
void fetch();
void decode();
void execute();
void mem();
void write_back();
int read_word(char *mem, unsigned int address);
void write_word(char *mem, unsigned int address, unsigned int data);

#endif
