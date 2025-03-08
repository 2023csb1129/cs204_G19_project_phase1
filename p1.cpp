

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <regex>

using namespace std;

// *R-Type Instructions*
struct RType {
    string opcode;
    string func3;
    string func7;
};

unordered_map<string, RType> rTypeInstructions = {
    {"add",  {"0110011", "000", "0000000"}},
    {"sub",  {"0110011", "000", "0100000"}},
    {"mul",  {"0110011", "000", "0000001"}},
};

// *I-Type Instructions*
struct IType {
    string opcode;
    string func3;
};

unordered_map<string, IType> iTypeInstructions = {
    {"addi", {"0010011", "000"}},
    {"andi", {"0010011", "111"}},
    {"ori",  {"0010011", "110"}},
    {"lb",   {"0000011", "000"}},
    {"lh",   {"0000011", "001"}},
    {"lw",   {"0000011", "010"}},
};

// *S-Type (Store) Instructions*
unordered_map<string, string> sTypeInstructions = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}
};

// *SB-Type (Branch) Instructions*
struct SBType {
    string opcode;
    string func3;
};

unordered_map<string, SBType> sbTypeInstructions = {
    {"beq", {"1100011", "000"}},
    {"bne", {"1100011", "001"}},
    {"bge", {"1100011", "101"}},
    {"blt", {"1100011", "100"}},
};

// *U-Type Instructions (lui, auipc)*
unordered_map<string, string> uTypeInstructions = {
    {"lui", "0110111"},
    {"auipc", "0010111"}
};

// *UJ-Type Instructions (jal)*
unordered_map<string, string> ujTypeInstructions = {
    {"jal", "1101111"}
};

// *Utility Functions*
string cleanToken(string token) {
    if (!token.empty() && token.back() == ',') {
        token.pop_back();
    }
    return token;
}

string getRegisterBinary(string reg) {
    reg = cleanToken(reg);
    if (reg.empty() || reg[0] != 'x') {
        cerr << "ERROR: Invalid register format: " << reg << endl;
        return "00000";
    }
    try {
        int regNum = stoi(reg.substr(1));
        return bitset<5>(regNum).to_string();
    } catch (...) {
        cerr << "ERROR: Invalid register: " << reg << endl;
        return "00000";
    }
}

string getImmediateBinary(string imm, int bits) {
    if (imm.empty()) {
        cerr << "ERROR: Missing immediate value!" << endl;
        return string(bits, '0');
    }
    try {
        int immValue = stoi(imm);
        return bitset<32>(immValue).to_string().substr(32 - bits, bits);
    } catch (...) {
        cerr << "ERROR: Invalid immediate: " << imm << endl;
        return string(bits, '0');
    }
}

// *Machine Code Generation*
string generateRType(string instr, string rd, string rs1, string rs2) {
    RType r = rTypeInstructions[instr];
    return r.func7 + getRegisterBinary(rs2) + getRegisterBinary(rs1) +
           r.func3 + getRegisterBinary(rd) + r.opcode;
}

string generateIType(string instr, string rd, string rs1, string imm) {
    return getImmediateBinary(imm, 12) + getRegisterBinary(rs1) +
           iTypeInstructions[instr].func3 + getRegisterBinary(rd) +
           iTypeInstructions[instr].opcode;
}

string generateSType(string instr, string rs2, string rs1, string imm) {
    string opcode = sTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 12);
    return immBinary.substr(0, 7) + getRegisterBinary(rs2) +
           getRegisterBinary(rs1) + "010" + immBinary.substr(7, 5) + opcode;
}

string generateSBType(string instr, string rs1, string rs2, string imm) {
    SBType sb = sbTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 12);
    return immBinary[0] + immBinary.substr(2, 6) + getRegisterBinary(rs2) +
           getRegisterBinary(rs1) + sb.func3 + immBinary.substr(8, 4) +
           immBinary[1] + sb.opcode;
}

string generateUType(string instr, string rd, string imm) {
    return getImmediateBinary(imm, 20) + getRegisterBinary(rd) + uTypeInstructions[instr];
}

// **UJ-Type Instruction Generation (jal)**
string generateUJType(string instr, string rd, string imm) {
    string immBinary = getImmediateBinary(imm, 20);
    return immBinary[0] + immBinary.substr(10, 10) + immBinary[9] +
           immBinary.substr(1, 8) + getRegisterBinary(rd) + ujTypeInstructions[instr];
}

// *Main Function*
int main() {
    ifstream input("input.asm");
    ofstream output("output.mc");

    string line;
    int address = 0;

    while (getline(input, line)) {
        istringstream iss(line);
        string instr, rd, rs1, rs2, imm, operand, machineCode;
        iss >> instr;

        if (instr.empty() || instr[0] == '#' || instr == ".text" || instr == ".data") continue;

        // *Handling R-Type Instructions*
        if (rTypeInstructions.count(instr)) {
            iss >> rd >> rs1 >> rs2;
            machineCode = generateRType(instr, cleanToken(rd), cleanToken(rs1), cleanToken(rs2));
        }

        // *Handling I-Type (Immediate & Load) Instructions*
        else if (iTypeInstructions.count(instr)) {
            iss >> rd >> operand;
            size_t pos = operand.find('(');
            if (pos != string::npos) {  // Load instruction (e.g., lb x1, 24(x2))
                imm = operand.substr(0, pos);
                rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
            } else {  // Immediate instruction (e.g., addi x1, x2, 10)
                rs1 = operand;
                iss >> imm;
            }
            machineCode = generateIType(instr, cleanToken(rd), cleanToken(rs1), cleanToken(imm));
        }

        // *Handling S-Type (Store) Instructions*
        else if (sTypeInstructions.count(instr)) {
            iss >> rs2 >> operand;
            size_t pos = operand.find('(');
            if (pos != string::npos) {
                imm = operand.substr(0, pos);
                rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
                machineCode = generateSType(instr, cleanToken(rs2), cleanToken(rs1), cleanToken(imm));
            } else {
                cerr << "ERROR: Invalid store instruction format: " << line << endl;
                continue;
            }
        }

        // *Handling SB-Type (Branch) Instructions*
        else if (sbTypeInstructions.count(instr)) {
            iss >> rs1 >> rs2 >> imm;
            machineCode = generateSBType(instr, cleanToken(rs1), cleanToken(rs2), cleanToken(imm));
        }

        // *Handling U-Type Instructions (lui, auipc)*
        else if (uTypeInstructions.count(instr)) {
            iss >> rd >> imm;
            machineCode = generateUType(instr, cleanToken(rd), cleanToken(imm));
        }

        // *Handling UJ-Type Instructions (jal)*
        else if (ujTypeInstructions.count(instr)) {
            iss >> rd >> imm;
            machineCode = generateUJType(instr, cleanToken(rd), cleanToken(imm));
        }

        // *Write to Output (Ensuring 8 Hex Digits)*
        if (!machineCode.empty()) {
            output << "0x" << hex << address << " 0x" << setw(8) << setfill('0') 
                   << stoul(machineCode, nullptr, 2) << " , " << line << " # " << machineCode << endl;
        }

        address += 4;
    }

    cout << "✅ Assembly successfully converted to machine code in output.mc!" << endl;
    return 0;
}







