
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <iomanip>

using namespace std;

// ✅ Global Label Table (for SB and UJ type instructions)
unordered_map<string, int> labelTable;

// ✅ Trim whitespace from strings
string trim(const string &s) {
    size_t first = s.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t");
    return s.substr(first, last - first + 1);
}

// ✅ Check if a string is a valid number
bool isNumber(const string &s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c) && c != '-') return false;
    }
    return true;
}

// ✅ Convert register string (e.g., "x5") to binary
string getRegisterBinary(string reg) {
    reg = trim(reg);
    if (reg[0] != 'x' || reg.size() < 2) {
        cerr << "Error: Invalid register " << reg << endl;
        return "00000";
    }
    int regNum = stoi(reg.substr(1));
    if (regNum < 0 || regNum > 31) {
        cerr << "Error: Register out of range " << reg << endl;
        return "00000";
    }
    return bitset<5>(regNum).to_string();
}

// ✅ Convert immediate value to binary with error handling
string getImmediateBinary(string imm, int bits) {
    try {
        int immValue = stoi(imm);
        return bitset<32>(immValue).to_string().substr(32 - bits, bits);
    } catch (...) {
        cerr << "Error: Invalid immediate value '" << imm << "'" << endl;
        return string(bits, '0');
    }
}

// ✅ Extract offset and register from memory format (e.g., `32(x14)`)
bool parseMemoryOperand(string operand, string &immediate, string &rs1) {
    size_t openParen = operand.find('(');
    size_t closeParen = operand.find(')');
    if (openParen == string::npos || closeParen == string::npos) {
        cerr << "Error: Invalid memory format " << operand << endl;
        return false;
    }

    immediate = operand.substr(0, openParen);
    rs1 = operand.substr(openParen + 1, closeParen - openParen - 1);

    if (!isNumber(immediate)) {
        cerr << "Error: Memory offset must be numeric " << immediate << endl;
        return false;
    }

    return true;
}

// ✅ Get PC-relative offset for SB and UJ type instructions safely
string getPCRelativeOffset(string label, int currentAddress, int bits) {
    if (labelTable.find(label) == labelTable.end()) {
        cerr << "Error: Undefined label '" << label << "'" << endl;
        return string(bits, '0');
    }
    int offset = labelTable[label] - currentAddress;
    return bitset<32>(offset).to_string().substr(32 - bits, bits);
}

// -------------------------------------------------
// ✅ Instruction Encoding Functions
// -------------------------------------------------

string encodeRType(string funct7, string rs2, string rs1, string funct3, string rd, string opcode) {
    return funct7 + rs2 + rs1 + funct3 + rd + opcode;
}

string encodeIType(string imm, string rs1, string funct3, string rd, string opcode) {
    return imm + rs1 + funct3 + rd + opcode;
}

string encodeSType(string imm, string rs2, string rs1, string funct3, string opcode) {
    string immHigh = imm.substr(0, 7);
    string immLow = imm.substr(7, 5);
    return immHigh + rs2 + rs1 + funct3 + immLow + opcode;
}

string encodeSBType(string imm, string rs2, string rs1, string funct3, string opcode) {
    string imm12 = imm.substr(0, 1);
    string imm10_5 = imm.substr(2, 6);
    string imm4_1 = imm.substr(8, 4);
    string imm11 = imm.substr(1, 1);
    return imm12 + imm10_5 + rs2 + rs1 + funct3 + imm4_1 + imm11 + opcode;
}

string encodeUType(string imm, string rd, string opcode) {
    return imm + rd + opcode;
}

string encodeUJType(string imm, string rd, string opcode) {
    string imm20 = imm.substr(0, 1);
    string imm10_1 = imm.substr(10, 10);
    string imm11 = imm.substr(9, 1);
    string imm19_12 = imm.substr(1, 8);
    return imm20 + imm19_12 + imm11 + imm10_1 + rd + opcode;
}

// -------------------------------------------------
// ✅ Main Function to Process Instructions
// -------------------------------------------------
int main() {
    ifstream input("input.asm");
    ofstream output("output.mc");

    if (!input || !output) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    string line;
    int address = 0;
    vector<string> instructions;

    // 🟢 First Pass: Read all lines & store labels
    while (getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line.back() == ':') {
            labelTable[line.substr(0, line.size() - 1)] = address;
        } else {
            instructions.push_back(line);
            address += 4;
        }
    }

    input.close();
    address = 0;

    // 🟢 Second Pass: Convert Instructions to Machine Code
    for (string instrLine : instructions) {
        istringstream iss(instrLine);
        string instr, rd, rs1, rs2, imm, operand;
        iss >> instr;

        string machineCode;

        if (instr == "add" || instr == "sub" || instr == "mul") {
            iss >> rd >> rs1 >> rs2;
            machineCode = encodeRType("0000000", getRegisterBinary(rs2), getRegisterBinary(rs1), "000", getRegisterBinary(rd), "0110011");
        } else if (instr == "addi" || instr == "andi" || instr == "ori") {
            iss >> rd >> rs1 >> imm;
            machineCode = encodeIType(getImmediateBinary(imm, 12), getRegisterBinary(rs1), "000", getRegisterBinary(rd), "0010011");
        } else if (instr == "sb" || instr == "sh" || instr == "sw" || instr == "sd") {
            iss >> rs2 >> operand;
            if (!parseMemoryOperand(operand, imm, rs1)) continue;
            machineCode = encodeSType(getImmediateBinary(imm, 12), getRegisterBinary(rs2), getRegisterBinary(rs1), "000", "0100011");
        } else if (instr == "beq" || instr == "bne" || instr == "blt" || instr == "bge") {
            iss >> rs1 >> rs2 >> imm;
            machineCode = encodeSBType(getPCRelativeOffset(imm, address, 13), getRegisterBinary(rs2), getRegisterBinary(rs1), "000", "1100011");
        } else if (instr == "lui" || instr == "auipc") {
            iss >> rd >> imm;
            machineCode = encodeUType(getImmediateBinary(imm, 20), getRegisterBinary(rd), "0110111");
        } else if (instr == "jal") {
            iss >> rd >> imm;
            machineCode = encodeUJType(getPCRelativeOffset(imm, address, 21), getRegisterBinary(rd), "1101111");
        }

        output << "0x" << hex << address << " 0x" << bitset<32>(stoi(machineCode, 0, 2)).to_ulong() << " , " << instrLine << endl;
        address += 4;
    }

    output.close();
    cout << "Assembly Successfully Converted!" << endl;
    return 0;
}
