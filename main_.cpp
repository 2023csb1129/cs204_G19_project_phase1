#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <regex>
#include <vector>

using namespace std;

// Structure for R-Type Instructions
struct RType {
    string opcode;
    string func3;
    string func7;
};

// R-Type Instructions Mapping
unordered_map<string, RType> rTypeInstructions = {
    {"add",  {"0110011", "000", "0000000"}},
    {"sub",  {"0110011", "000", "0100000"}},
    {"and",  {"0110011", "111", "0000000"}},
    {"or",   {"0110011", "110", "0000000"}},
    {"xor",  {"0110011", "100", "0000000"}},
    {"sll",  {"0110011", "001", "0000000"}},
    {"srl",  {"0110011", "101", "0000000"}},
    {"sra",  {"0110011", "101", "0100000"}},
    {"slt",  {"0110011", "010", "0000000"}},
    {"mul",  {"0110011", "000", "0000001"}},
    {"div",  {"0110011", "100", "0000001"}},
    {"rem",  {"0110011", "110", "0000001"}}
};

// I-Type Instructions Mapping
struct IType {
    string opcode;
    string func3;
};

unordered_map<string, IType> iTypeInstructions = {
    {"addi", {"0010011", "000"}},
    {"andi", {"0010011", "111"}},
    {"ori",  {"0010011", "110"}},
    {"jalr", {"1100111", "000"}},
    {"lb",   {"0000011", "000"}},
    {"lh",   {"0000011", "001"}},
    {"lw",   {"0000011", "010"}},
    {"ld",   {"0000011", "011"}}
};

// S-Type Instructions Mapping (Store)
unordered_map<string, string> sTypeInstructions = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}
};

// SB-Type Instructions Mapping (Branch)
unordered_map<string, string> sbTypeInstructions = {
    {"beq", "1100011"}, {"bne", "1100011"}, {"bge", "1100011"}, {"blt", "1100011"}
};

// U-Type Instructions Mapping (Upper immediate)
unordered_map<string, string> uTypeInstructions = {
    {"lui", "0110111"}, {"auipc", "0010111"}
};

// UJ-Type Instructions Mapping (Jump)
unordered_map<string, string> ujTypeInstructions = {
    {"jal", "1101111"}
};

// Helper Functions

// Remove trailing comma from a token
string cleanToken(string token) {
    if (!token.empty() && token.back() == ',') {
        token.pop_back();
    }
    return token;
}

// Convert register name (x0-x31) to binary representation
string getRegisterBinary(string reg) {
    reg = cleanToken(reg);
    if (reg.size() < 2 || reg[0] != 'x') return "00000";  // Return binary of x0 if invalid
    return bitset<5>(stoi(reg.substr(1))).to_string();
}

// Convert immediate value to binary with fixed sign extension
string getImmediateBinary(string imm, int bits) {
    int immValue = stoi(imm);
    if (bits == 12) {
        return bitset<12>(immValue & 0xFFF).to_string();
    } else if (bits == 5) {
        return bitset<5>(immValue & 0x1F).to_string();
    } else {
        return bitset<32>(immValue).to_string();
    }
}

// Generate R-Type machine code
string generateRType(string instr, string rd, string rs1, string rs2) {
    RType rType = rTypeInstructions[instr];
    return rType.func7 + getRegisterBinary(rs2) + getRegisterBinary(rs1) + rType.func3 + getRegisterBinary(rd) + rType.opcode;
}

// Generate I-Type machine code
string generateIType(string instr, string rd, string rs1, string imm) {
    IType iType = iTypeInstructions[instr];
    return getImmediateBinary(imm, 12) + getRegisterBinary(rs1) + iType.func3 + getRegisterBinary(rd) + iType.opcode;
}

// Generate S-Type machine code
string generateSType(string instr, string rs2, string rs1, string imm) {
    string opcode = sTypeInstructions[instr];
    return getImmediateBinary(imm, 12).substr(0, 7) + getRegisterBinary(rs2) + getRegisterBinary(rs1) + getImmediateBinary(imm, 12).substr(7, 5) + opcode;
}

// Generate SB-Type machine code
string generateSBType(string instr, string rs1, string rs2, string imm) {
    string opcode = sbTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 12);
    return immBinary.substr(0, 7) + getRegisterBinary(rs2) + getRegisterBinary(rs1) + "000" + immBinary.substr(7, 5) + opcode;
}

// Generate U-Type machine code
string generateUType(string instr, string rd, string imm) {
    string opcode = uTypeInstructions[instr];
    return getImmediateBinary(imm, 20) + getRegisterBinary(rd) + opcode;
}

// Generate UJ-Type machine code
string generateUJType(string instr, string rd, string imm) {
    string opcode = ujTypeInstructions[instr];
    return getImmediateBinary(imm, 21) + getRegisterBinary(rd) + opcode;
}

// Main Assembler Function
int main() {
    ifstream input("input.asm");  // Read assembly input file
    ofstream output("output.mc"); // Write machine code output file

    string line;
    int address = 0;  // Start at the beginning of the text segment (0x00000000)

    while (getline(input, line)) {
        istringstream iss(line);
        string instr, rd, rs1, rs2, imm, operand;
        string machineCode;

        iss >> instr;
        if (instr.empty() || instr[0] == '#' || instr == ".text" || instr == ".data") continue;  // Ignore comments and directives

        if (rTypeInstructions.count(instr)) {  // R-Type instruction
            iss >> rd >> rs1 >> rs2;
            rd = cleanToken(rd); rs1 = cleanToken(rs1); rs2 = cleanToken(rs2);
            machineCode = generateRType(instr, rd, rs1, rs2);
        } else if (iTypeInstructions.count(instr)) {  // I-Type instruction
            iss >> rd >> rs1 >> imm;
            rd = cleanToken(rd); rs1 = cleanToken(rs1);
            machineCode = generateIType(instr, rd, rs1, imm);
        } else if (sTypeInstructions.count(instr)) {  // S-Type instruction
            iss >> rs2 >> operand;
            rs2 = cleanToken(rs2);
            machineCode = generateSType(instr, rs2, operand, imm);
        } else if (sbTypeInstructions.count(instr)) {  // SB-Type instruction
            iss >> rs1 >> rs2 >> imm;
            rs1 = cleanToken(rs1); rs2 = cleanToken(rs2);
            machineCode = generateSBType(instr, rs1, rs2, imm);
        } else if (uTypeInstructions.count(instr)) {  // U-Type instruction
            iss >> rd >> imm;
            rd = cleanToken(rd);
            machineCode = generateUType(instr, rd, imm);
        } else if (ujTypeInstructions.count(instr)) {  // UJ-Type instruction
            iss >> rd >> imm;
            rd = cleanToken(rd);
            machineCode = generateUJType(instr, rd, imm);
        } else {
            cerr << "ERROR: Unsupported instruction: " << instr << endl;
            continue;
        }

        // Output the machine code to the file
        output << "0x" << hex << address << " 0x" << hex << stoul(machineCode, nullptr, 2)
               << " , " << line << " # " << machineCode << endl;
        address += 4;  // Increment address (4 bytes per instruction)
    }

    return 0;
}
