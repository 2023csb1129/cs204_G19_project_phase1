#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <regex>

using namespace std;

// R-Type Instructions
struct RType
{
    string opcode;
    string func3;
    string func7;
};

unordered_map<string, RType> rTypeInstructions = {
    {"add", {"0110011", "000", "0000000"}},
    {"sub", {"0110011", "000", "0100000"}},
    {"and", {"0110011", "111", "0000000"}},
    {"or", {"0110011", "110", "0000000"}},
    {"xor", {"0110011", "100", "0000000"}},
    {"sll", {"0110011", "001", "0000000"}},
    {"srl", {"0110011", "101", "0000000"}},
    {"sra", {"0110011", "101", "0100000"}},
    {"slt", {"0110011", "010", "0000000"}},
    {"mul", {"0110011", "000", "0000001"}},
    {"div", {"0110011", "100", "0000001"}},
    {"rem", {"0110011", "110", "0000001"}},
};

// I-Type Instructions
struct IType
{
    string opcode;
    string func3;
};

unordered_map<string, IType> iTypeInstructions = {
    {"addi", {"0010011", "000"}},
    {"andi", {"0010011", "111"}},
    {"ori", {"0010011", "110"}},
    {"jalr", {"1100111", "000"}},
    {"lb", {"0000011", "000"}},
    {"lh", {"0000011", "001"}},
    {"lw", {"0000011", "010"}},
    {"ld", {"0000011", "011"}},
};

// S-Type Instructions (Store)
unordered_map<string, string> sTypeInstructions = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}};

// Function to clean token (remove trailing comma)
string cleanToken(string token)
{
    if (!token.empty() && token.back() == ',')
    {
        token.pop_back();
    }
    return token;
}

// Function to parse memory operand format: `32(x14)`
bool parseMemoryOperand(string operand, string &immediate, string &rs1)
{
    regex memPattern(R"(([-]?\d+)\((x\d+)\))");
    smatch match;
    if (regex_match(operand, match, memPattern))
    {
        immediate = match[1].str();
        rs1 = match[2].str();
        return true;
    }
    return false;
}

// Function to convert register name (x0-x31) to binary
string getRegisterBinary(string reg)
{
    reg = cleanToken(reg);
    if (reg.size() < 2 || reg[0] != 'x')
        return "00000";
    return bitset<5>(stoi(reg.substr(1))).to_string();
}

// Function to convert immediate value (Fixed sign extension)
string getImmediateBinary(string imm, int bits)
{
    int immValue = stoi(imm);
    if (bits == 12)
        return bitset<12>(immValue & 0xFFF).to_string();
    else if (bits == 5)
        return bitset<5>(immValue & 0x1F).to_string();
    else
        return bitset<32>(immValue).to_string();
}

// Function to generate R-type machine code
string generateRType(string instr, string rd, string rs1, string rs2)
{
    RType r = rTypeInstructions[instr];
    return r.func7 + getRegisterBinary(rs2) + getRegisterBinary(rs1) + r.func3 + getRegisterBinary(rd) + r.opcode;
}

// Function to generate I-type machine code
string generateIType(string instr, string rd, string rs1, string imm)
{
    IType i = iTypeInstructions[instr];
    return getImmediateBinary(imm, 12) + getRegisterBinary(rs1) + i.func3 + getRegisterBinary(rd) + i.opcode;
}

// Function to generate I-type Load machine code
string generateITypeLoad(string instr, string rd, string rs1, string imm)
{
    return getImmediateBinary(imm, 12) + getRegisterBinary(rs1) + iTypeInstructions[instr].func3 + getRegisterBinary(rd) + iTypeInstructions[instr].opcode;
}

// Function to generate S-type machine code
string generateSType(string instr, string rs2, string rs1, string imm)
{
    string opcode = sTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 12);
    return immBinary.substr(0, 7) + getRegisterBinary(rs2) + getRegisterBinary(rs1) + "010" + immBinary.substr(7, 5) + opcode;
}

// Main function
int main()
{
    ifstream input("input.asm");
    ofstream output("output.mc");

    string line;
    int address = 0;

    while (getline(input, line))
    {
        istringstream iss(line);
        string instr, rd, rs1, rs2, imm, operand;
        string machineCode;

        iss >> instr;
        if (instr.empty() || instr[0] == '#' || instr == ".text" || instr == ".data")
            continue;

        cout << "DEBUG: Processing instruction: " << instr << endl;

        if (rTypeInstructions.count(instr))
        {
            iss >> rd >> rs1 >> rs2;
            rd = cleanToken(rd);
            rs1 = cleanToken(rs1);
            rs2 = cleanToken(rs2);
            cout << "DEBUG: R-Type " << instr << " - rd=" << rd << ", rs1=" << rs1 << ", rs2=" << rs2 << endl;
            machineCode = generateRType(instr, rd, rs1, rs2);
        }
        else if (iTypeInstructions.count(instr))
        {
            if (instr == "lb" || instr == "lh" || instr == "lw" || instr == "ld")
            {
                iss >> rd >> operand;
                rd = cleanToken(rd);
                if (!parseMemoryOperand(operand, imm, rs1))
                {
                    cerr << "ERROR: Invalid memory format in " << line << endl;
                    continue;
                }
                cout << "DEBUG: I-Type LOAD " << instr << " - rd=" << rd << ", rs1=" << rs1 << ", imm=" << imm << endl;
                machineCode = generateITypeLoad(instr, rd, rs1, imm);
            }
            else
            {
                iss >> rd >> rs1 >> imm;
                rd = cleanToken(rd);
                rs1 = cleanToken(rs1);
                cout << "DEBUG: I-Type " << instr << " - rd=" << rd << ", rs1=" << rs1 << ", imm=" << imm << endl;
                machineCode = generateIType(instr, rd, rs1, imm);
            }
        }
        else if (sTypeInstructions.count(instr))
        {
            iss >> rs2 >> operand;
            rs2 = cleanToken(rs2);
            if (!parseMemoryOperand(operand, imm, rs1))
            {
                cerr << "ERROR: Invalid memory format in " << line << endl;
                continue;
            }
            cout << "DEBUG: S-Type STORE " << instr << " - rs2=" << rs2 << ", rs1=" << rs1 << ", imm=" << imm << endl;
            machineCode = generateSType(instr, rs2, rs1, imm);
        }
        else
        {
            cerr << "ERROR: Unsupported instruction: " << instr << endl;
            continue;
        }

        output << "0x" << hex << address << " 0x" << hex << stoul(machineCode, nullptr, 2) << " , " << line << " # " << machineCode << endl;
        address += 4;
    }

    return 0;
}