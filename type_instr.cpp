#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <vector>
#include <cctype>
#include <cstdlib>

using namespace std;

// --------------------- Instruction Maps --------------------- //
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
    {"mul", {"0110011", "000", "0000001"}},
    {"div", {"0110011", "100", "0000001"}},
    {"rem", {"0110011", "110", "0000001"}},
    {"and", {"0110011", "111", "0000000"}},
    {"or", {"0110011", "110", "0000000"}},
    {"xor", {"0110011", "100", "0000000"}},
    {"sll", {"0110011", "001", "0000000"}},
    {"srl", {"0110011", "101", "0000000"}},
    {"sra", {"0110011", "101", "0100000"}},
    {"slt", {"0110011", "010", "0000000"}}};

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
    {"lb", {"0000011", "000"}},
    {"lh", {"0000011", "001"}},
    {"lw", {"0000011", "010"}},
    {"ld", {"0000011", "011"}},
    {"jalr", {"1100111", "000"}}};

// S-Type (Store) Instructions
unordered_map<string, string> sTypeInstructions = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}};

// SB-Type (Branch) Instructions
struct SBType
{
    string opcode;
    string func3;
};
unordered_map<string, SBType> sbTypeInstructions = {
    {"beq", {"1100011", "000"}},
    {"bne", {"1100011", "001"}},
    {"bge", {"1100011", "101"}},
    {"blt", {"1100011", "100"}}};

// U-Type Instructions (lui, auipc)
unordered_map<string, string> uTypeInstructions = {
    {"lui", "0110111"},
    {"auipc", "0010111"}};

// UJ-Type Instructions (jal)
unordered_map<string, string> ujTypeInstructions = {
    {"jal", "1101111"}};

// ------------------ Utility Functions ------------------ //
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
    if (reg.empty() || reg[0] != 'x')
    {
        cerr << "ERROR: Invalid register format: " << reg << endl;
        return "00000";
    }
    try
    {
        int regNum = stoi(reg.substr(1));
        return bitset<5>(regNum).to_string();
    }
    catch (...)
    {
        cerr << "ERROR: Invalid register: " << reg << endl;
        return "00000";
    }
}

string getImmediateBinary(string imm, int bits)
{
    try
    {
        int immValue = stoi(imm);
        // Convert to unsigned using two's complement for negative values.
        unsigned int uVal = static_cast<unsigned int>(immValue) & ((1u << bits) - 1);
        return bitset<32>(uVal).to_string().substr(32 - bits, bits);
    }
    catch (...)
    {
        cerr << "ERROR: Invalid immediate: " << imm << endl;
        return string(bits, '0');
    }
}

// ------------------ Instruction Encoding Functions ------------------ //
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
    string imm11_5 = immBinary.substr(0, 7);
    string imm4_0 = immBinary.substr(7, 5);

    string func3;
    if (instr == "sb")
        func3 = "000";
    if (instr == "sh")
        func3 = "001";
    if (instr == "sw")
        func3 = "010";
    if (instr == "sd")
        func3 = "011";

    return imm11_5 + getRegisterBinary(rs2) + getRegisterBinary(rs1) +
           func3 + imm4_0 + opcode;
}

string generateSBType(string instr, string rs1, string rs2, string imm)
{
    SBType sb = sbTypeInstructions[instr];
    string immBinary = getImmediateBinary(imm, 13);
    // RISC-V branch encoding: imm[12] | imm[10:5] | rs2 | rs1 | func3 | imm[4:1] | imm[11] | opcode
    return string(1, immBinary[0]) + immBinary.substr(2, 6) + getRegisterBinary(rs2) +
           getRegisterBinary(rs1) + sb.func3 + immBinary.substr(8, 4) +
           string(1, immBinary[1]) + sb.opcode;
}

string generateUType(string instr, string rd, string imm)
{
    return getImmediateBinary(imm, 20) + getRegisterBinary(rd) + uTypeInstructions[instr];
}

string generateUJType(string instr, string rd, string imm)
{
    // Use the full byte offset (e.g. for a forward jump from address 0x0 to 0x8, offset = 8,
    // for a backward jump from 0x4 to 0x0, offset = -4)
    int offset = stoi(imm);
    // Convert to a 21-bit two's complement value.
    uint32_t imm21 = static_cast<uint32_t>(offset) & ((1u << 21) - 1);

    // Extract fields according to the JAL format:
    //   imm[20]    -> bit 31
    //   imm[10:1]  -> bits 30:21
    //   imm[11]    -> bit 20
    //   imm[19:12] -> bits 19:12
    uint32_t bit20 = (imm21 >> 20) & 0x1;
    uint32_t bits10_1 = (imm21 >> 1) & 0x3FF;
    uint32_t bit11 = (imm21 >> 11) & 0x1;
    uint32_t bits19_12 = (imm21 >> 12) & 0xFF;

    int rdNum = stoi(rd.substr(1));
    uint32_t machineCode = (bit20 << 31) |
                           (bits10_1 << 21) |
                           (bit11 << 20) |
                           (bits19_12 << 12) |
                           (rdNum << 7) |
                           0x6F; // opcode for jal
    return bitset<32>(machineCode).to_string();
}

/*string generateUJType(string instr, string rd, string imm) {
    int immValue = stoi(imm);
    // Extract fields according to RISC‑V JAL encoding:
    // The jump offset (imm) is a 21-bit signed value.
    // The encoded immediate is arranged as:
    //   imm[20]   : bit31
    //   imm[10:1] : bits30-21
    //   imm[11]   : bit20
    //   imm[19:12]: bits19-12
    int imm20    = (immValue >> 20) & 0x1;
    int imm10_1  = (immValue >> 1) & 0x3FF;  // 10 bits for bits 10:1
    int imm11    = (immValue >> 11) & 0x1;
    int imm19_12 = (immValue >> 12) & 0xFF;   // 8 bits for bits 19:12

    // Now build the 32-bit machine code using bit shifts:
    // rd is extracted from the register number (assume rd is like "x0")
    int rdNum = stoi(rd.substr(1));
    unsigned int machineCode = (imm20    << 31) |
                               (imm10_1  << 21) |
                               (imm11    << 20) |
                               (imm19_12 << 12) |
                               (rdNum    << 7)  |
                                0x6F;  // opcode for jal
    // Convert machine code to an 8-digit hexadecimal string
    return bitset<32>(machineCode).to_string();
}
*/

// ------------------ Label & Directive Support ------------------ //
enum Segment
{
    NONE,
    TEXT,
    DATA
};
unordered_map<string, unsigned int> symbolTable; // label -> absolute address
vector<string> fileLines;                        // stores all lines for two-pass processing

bool isNumber(const string &s)
{
    if (s.empty())
        return false;
    size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    for (; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}
// --- New Helper Functions for Trimming and Parsing Data Values --- //

// Trim leading and trailing whitespace from a string.
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}
unsigned int parseDataDirectiveValue(const string &token)
{
    string trimmed = trim(token);
    unsigned int val = 0;
    if (trimmed.substr(0, 2) == "0x" || trimmed.substr(0, 2) == "0X")
        val = stoul(trimmed, nullptr, 16);
    else
        val = stoul(trimmed);
    return val;
}

// ------------------ Main Function ------------------ //
int main()
{
    ifstream input("input.asm");
    if (!input)
    {
        cerr << "Could not open input.asm" << endl;
        return 1;
    }

    // Read all lines from input.asm
    string line;
    while (getline(input, line))
    {
        fileLines.push_back(line);
    }
    input.close();

    // First Pass: Build symbol table and compute addresses.
    Segment currSegment = NONE;
    unsigned int currAddress = 0;
    for (auto &l : fileLines)
    {
        if (l.empty() || l[0] == '#')
            continue;
        istringstream iss(l);
        string token;
        iss >> token;
        // Pointer to current stream.
        istringstream *currStream = &iss;
        string directivePart;
        if (token.back() == ':')
        {
            string label = token.substr(0, token.size() - 1);
            symbolTable[label] = currAddress;
            getline(iss, directivePart);
            if (directivePart.empty())
                continue;
            // Use a new stream for the rest of the line.
            currStream = new istringstream(directivePart);
            (*currStream) >> token;
        }
        if (token == ".text")
        {
            currSegment = TEXT;
            currAddress = 0;
        }
        else if (token == ".data")
        {
            currSegment = DATA;
            currAddress = 0x10000000;
        }
        else if (currSegment == TEXT)
        {
            // Each text instruction is 4 bytes.
            currAddress += 4;
        }
        else if (currSegment == DATA)
        {
            if (token == ".byte")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (!value.empty())
                        currAddress += 1;
                }
            }
            else if (token == ".half")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (!value.empty())
                        currAddress += 2;
                }
            }
            else if (token == ".word")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (!value.empty())
                        currAddress += 4;
                }
            }
            else if (token == ".dword")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (!value.empty())
                        currAddress += 8;
                }
            }
            else if (token == ".asciz")
            {
                string str;
                getline(*currStream, str);
                size_t start = str.find('\"');
                size_t end = str.rfind('\"');
                if (start != string::npos && end != string::npos && end > start)
                {
                    string content = str.substr(start + 1, end - start - 1);
                    currAddress += content.size() + 1;
                }
            }
        }
        if (currStream != &iss)
            delete currStream;
    }

    // Second Pass: Generate output code (text and data segments)
    ofstream output("output.mc");
    if (!output)
    {
        cerr << "Could not open output.mc for writing" << endl;
        return 1;
    }

    currSegment = NONE;
    unsigned int textAddr = 0;          // text segment starting address
    unsigned int dataAddr = 0x10000000; // data segment starting address
    unsigned int currTextAddr = 0;
    unsigned int currDataAddr = dataAddr;
    bool textSegmentEnded = false;

    vector<string> textOutput;
    vector<string> dataOutput;

    for (auto &l : fileLines)
    {
        if (l.empty() || l[0] == '#')
            continue;
        istringstream iss(l);
        string token;
        iss >> token;
        istringstream *currStream = &iss;
        string directivePart;
        if (token.back() == ':')
        {
            // Remove label.
            getline(iss, directivePart);
            if (directivePart.empty())
                continue;
            currStream = new istringstream(directivePart);
            (*currStream) >> token;
        }

        if (token == ".text")
        {
            currSegment = TEXT;
            currTextAddr = 0;
            continue;
        }
        else if (token == ".data")
        {
            if (currSegment == TEXT && !textSegmentEnded)
            {
                // Termination code for text segment.
                ostringstream oss;
                oss << "0x" << hex << currTextAddr << " 0xFFFFFFFF , END";
                textOutput.push_back(oss.str());
                textSegmentEnded = true;
            }
            currSegment = DATA;
            currDataAddr = dataAddr;
            continue;
        }

        if (currSegment == TEXT)
        {
            string machineCode;
            if (rTypeInstructions.count(token))
            {
                string rd, rs1, rs2;
                (*currStream) >> rd >> rs1 >> rs2;
                machineCode = generateRType(token, cleanToken(rd), cleanToken(rs1), cleanToken(rs2));
            }
            else if (iTypeInstructions.count(token))
            {
                string rd, operand;
                (*currStream) >> rd >> operand;
                string imm, rs1;
                size_t pos = operand.find('(');
                if (pos != string::npos)
                {
                    imm = operand.substr(0, pos);
                    rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
                }
                else
                {
                    rs1 = operand;
                    (*currStream) >> imm;
                }
                machineCode = generateIType(token, cleanToken(rd), cleanToken(rs1), cleanToken(imm));
            }
            else if (sTypeInstructions.count(token))
            {
                string rs2, operand;
                (*currStream) >> rs2 >> operand;
                string imm, rs1;
                size_t pos = operand.find('(');
                if (pos != string::npos)
                {
                    imm = operand.substr(0, pos);
                    rs1 = operand.substr(pos + 1, operand.find(')') - pos - 1);
                }
                machineCode = generateSType(token, cleanToken(rs2), cleanToken(rs1), cleanToken(imm));
            }
            else if (sbTypeInstructions.count(token))
            {
                string rs1, rs2, imm;
                (*currStream) >> rs1 >> rs2 >> imm;
                // Compute PC-relative offset for branch (divide by 2)
                // For branch instructions (SB-type)
                if (!isNumber(imm))
                {
                    int offset = symbolTable[imm] - currTextAddr; // use full byte offset
                    imm = to_string(offset);
                }

                machineCode = generateSBType(token, cleanToken(rs1), cleanToken(rs2), cleanToken(imm));
            }
            else if (uTypeInstructions.count(token))
            {
                string rd, imm;
                (*currStream) >> rd >> imm;
                machineCode = generateUType(token, cleanToken(rd), cleanToken(imm));
            }

            else if (ujTypeInstructions.count(token))
            {
                string rd, label;
                iss >> rd >> label;
                int offset = symbolTable[label] - currTextAddr; // full byte offset, e.g. 8 or -4
                string imm = to_string(offset);                 // pass the full offset!
                machineCode = generateUJType(token, cleanToken(rd), cleanToken(imm));
            }

            else
            {
                if (currStream != &iss)
                    delete currStream;
                continue;
            }
            ostringstream oss;
            oss << "0x" << hex << currTextAddr << " 0x"
                << setw(8) << setfill('0') << stoul(machineCode, nullptr, 2)
                << " , " << l << " # " << machineCode;
            textOutput.push_back(oss.str());
            currTextAddr += 4;
        }
        else if (currSegment == DATA)
        {
            if (token == ".byte")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (value.empty())
                        continue;
                    unsigned int byteVal = parseDataDirectiveValue(value);
                    ostringstream oss;
                    oss << "0x" << hex << currDataAddr << " 0x"
                        << setw(2) << setfill('0') << byteVal;
                    dataOutput.push_back(oss.str());
                    currDataAddr += 1;
                }
            }
            else if (token == ".half")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (value.empty())
                        continue;
                    unsigned int halfVal = parseDataDirectiveValue(value);
                    ostringstream oss;
                    oss << "0x" << hex << currDataAddr << " 0x"
                        << setw(4) << setfill('0') << halfVal;
                    dataOutput.push_back(oss.str());
                    currDataAddr += 2;
                }
            }
            else if (token == ".word")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (value.empty())
                        continue;
                    unsigned int wordVal = parseDataDirectiveValue(value);
                    ostringstream oss;
                    oss << "0x" << hex << currDataAddr << " 0x"
                        << setw(8) << setfill('0') << wordVal;
                    dataOutput.push_back(oss.str());
                    currDataAddr += 4;
                }
            }
            else if (token == ".dword")
            {
                string values;
                getline(*currStream, values);
                istringstream issValues(values);
                string value;
                while (getline(issValues, value, ','))
                {
                    if (value.empty())
                        continue;
                    unsigned long long dwordVal = stoull(value, nullptr, 0);
                    ostringstream oss;
                    oss << "0x" << hex << currDataAddr << " 0x"
                        << setw(16) << setfill('0') << dwordVal;
                    dataOutput.push_back(oss.str());
                    currDataAddr += 8;
                }
            }
            else if (token == ".asciz")
            {
                string str;
                getline(*currStream, str);
                size_t start = str.find('\"');
                size_t end = str.rfind('\"');
                if (start != string::npos && end != string::npos && end > start)
                {
                    string content = str.substr(start + 1, end - start - 1);
                    for (char c : content)
                    {
                        ostringstream oss;
                        oss << "0x" << hex << currDataAddr << " 0x"
                            << setw(2) << setfill('0') << (int)c;
                        dataOutput.push_back(oss.str());
                        currDataAddr += 1;
                    }
                    // Null terminator.
                    ostringstream oss;
                    oss << "0x" << hex << currDataAddr << " 0x00";
                    dataOutput.push_back(oss.str());
                    currDataAddr += 1;
                }
            }
        }
        if (currStream != &iss)
            delete currStream;
    }

    // Write text segment first.
    for (auto &s : textOutput)
    {
        output << s << "\n";
    }
    output << "\n"; // Separate segments.
    for (auto &s : dataOutput)
    {
        output << s << "\n";
    }

    output.close();
    cout << "✅ Assembly successfully converted to machine code in output.mc!" << endl;
    return 0;
}
