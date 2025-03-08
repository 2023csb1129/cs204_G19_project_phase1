#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>

using namespace std;

// **R-Type Instructions**
struct RType
{
    string opcode;
    string func3;
    string func7;
};

unordered_map<string, RType> rTypeInstructions = {
    {"add", {"0110011", "000", "0000000"}},
    {"sub", {"0110011", "000", "0100000"}},
    {"mul", {"0110011", "000", "0000001"}},
};

// **I-Type Instructions**
struct IType
{
    string opcode;
    string func3;
};

unordered_map<string, IType> iTypeInstructions = {
    {"addi", {"0010011", "000"}},
    {"andi", {"0010011", "111"}},
    {"ori", {"0010011", "110"}},
    {"lb", {"0000011", "000"}},
    {"lh", {"0000011", "001"}},
    {"lw", {"0000011", "010"}},
};

// **S-Type Instructions**
unordered_map<string, string> sTypeInstructions = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}};

// **U-Type Instructions**
unordered_map<string, string> uTypeInstructions = {
    {"lui", "0110111"},
    {"auipc", "0010111"}};

// **Utility Functions**
string cleanToken(string token)
{
    if (!token.empty() && token.back() == ',')
    {
        token.pop_back();
    }
    return token;
}

string getRegisterBinary(string reg)
{
    reg = cleanToken(reg);
    if (reg[0] != 'x')
        return "00000";
    return bitset<5>(stoi(reg.substr(1))).to_string();
}

string getImmediateBinary(string imm, int bits)
{
    return bitset<32>(stoi(imm)).to_string().substr(32 - bits, bits);
}

// **Machine Code Generation**
string generateRType(string instr, string rd, string rs1, string rs2)
{
    RType r = rTypeInstructions[instr];
    return r.func7 + getRegisterBinary(rs2) + getRegisterBinary(rs1) +
           r.func3 + getRegisterBinary(rd) + r.opcode;
}

string generateIType(string instr, string rd, string rs1, string imm)
{
    return getImmediateBinary(imm, 12) + getRegisterBinary(rs1) +
           iTypeInstructions[instr].func3 + getRegisterBinary(rd) +
           iTypeInstructions[instr].opcode;
}

string generateSType(string instr, string rs2, string rs1, string imm)
{
    string opcode = sTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 12);
    return immBinary.substr(0, 7) + getRegisterBinary(rs2) +
           getRegisterBinary(rs1) + "010" + immBinary.substr(7, 5) + opcode;
}

string generateUType(string instr, string rd, string imm)
{
    return getImmediateBinary(imm, 20) + getRegisterBinary(rd) + uTypeInstructions[instr];
}

// **Main Function**
int main()
{
    ifstream input("input.txt"); // 🔹 Changed input file to input.txt
    ofstream output("output.mc");

    if (!input.is_open())
    {
        cerr << " ERROR: Unable to open input.txt!" << endl;
        return 1;
    }
    if (!output.is_open())
    {
        cerr << " ERROR: Unable to create output.mc!" << endl;
        return 1;
    }

    string line;
    int address = 0;

    while (getline(input, line))
    {
        istringstream iss(line);
        string instr, rd, rs1, rs2, imm, operand, machineCode;
        iss >> instr;

        if (instr.empty() || instr[0] == '#' || instr == ".text" || instr == ".data")
            continue;

        // **Handling R-Type Instructions**
        if (rTypeInstructions.count(instr))
        {
            iss >> rd >> rs1 >> rs2;
            machineCode = generateRType(instr, cleanToken(rd), cleanToken(rs1), cleanToken(rs2));
        }

        // **Handling I-Type Instructions**
        else if (iTypeInstructions.count(instr))
        {
            iss >> rd >> operand;
            size_t pos = operand.find('(');
            if (pos != string::npos)
            {
                imm = operand.substr(0, pos);
                rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
            }
            else
            {
                rs1 = operand;
                iss >> imm;
            }
            machineCode = generateIType(instr, cleanToken(rd), cleanToken(rs1), cleanToken(imm));
        }

        // **Handling S-Type (Store) Instructions**
        else if (sTypeInstructions.count(instr))
        {
            iss >> rs2 >> operand;
            size_t pos = operand.find('(');
            if (pos != string::npos)
            {
                imm = operand.substr(0, pos);
                rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
                machineCode = generateSType(instr, cleanToken(rs2), cleanToken(rs1), cleanToken(imm));
            }
        }

        // **Handling U-Type Instructions**
        else if (uTypeInstructions.count(instr))
        {
            iss >> rd >> imm;
            machineCode = generateUType(instr, cleanToken(rd), cleanToken(imm));
        }

        // **Write to Output File**
        if (!machineCode.empty())
        {
            output << "0x" << hex << address << " 0x" << hex << stoul(machineCode, nullptr, 2)
                   << " , " << line << " # " << machineCode << endl;
        }

        address += 4;
    }

    cout << "Conversion completed! Check output.mc" << endl;
    return 0;
}
