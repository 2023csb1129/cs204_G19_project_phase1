// this is for storing the opcode, funct3, and funct7 for R-type instucrtions
struct R_t {
    string op;
    string f3;
    string f7;
};

//here we are matching r-type instructions with their respcetive binary notations
unordered_map<string, R_t> instrmap_r = {
    {"add",  {"0110011", "000", "0000000"}},
    {"sub",  {"0110011", "000", "0100000"}},
    {"mul",  {"0110011", "000", "0000001"}},
    {"div",  {"0110011", "100", "0000001"}},
    {"rem",  {"0110011", "110", "0000001"}},
    {"and",  {"0110011", "111", "0000000"}},
    {"or",   {"0110011", "110", "0000000"}},
    {"xor",  {"0110011", "100", "0000000"}},
    {"sll",  {"0110011", "001", "0000000"}},
    {"srl",  {"0110011", "101", "0000000"}},
    {"sra",  {"0110011", "101", "0100000"}},
    {"slt",  {"0110011", "010", "0000000"}}
};

// this is for storing the opcode and funct3 for I-type instructions
struct I_t {
    string op;
    string f3;
};

// as mentioned for above that is for r-type instructions similarly do here for i-type .
unordered_map<string, I_t> instrmap_i = {
    {"addi", {"0010011", "000"}},
    {"andi", {"0010011", "111"}},
    {"ori",  {"0010011", "110"}},
    {"lb",   {"0000011", "000"}},
    {"lh",   {"0000011", "001"}},
    {"lw",   {"0000011", "010"}},
    {"ld",   {"0000011", "011"}},
    {"jalr", {"1100111", "000"}}
};

//storing the opcode and funct3 for branch instructions
struct B_t {
    string op;
    string f3;
};

// again here mapping branch instructions.
unordered_map<string, B_t> instrmap_br = {
    {"beq", {"1100011", "000"}},
    {"bne", {"1100011", "001"}},
    {"bge", {"1100011", "101"}},
    {"blt", {"1100011", "100"}}
};

// Mapping for store instructions (S-type); allof them use the same opcode.
unordered_map<string, string> instrmap_st = {
    {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"}
};

// Mapping for UJ-type instructions here jal
unordered_map<string, string> instrmap_uj = {
    {"jal", "1101111"}
};

// Mapping for U-type instructions like lui and auipc
unordered_map<string, string> instrmap_u = {
    {"lui", "0110111"},
    {"auipc", "0010111"}
};

//Removes the last character from a string if it is a comma
string rm_comma(string tok) {
    if (!tok.empty() && tok.back() == ',') {
        tok.pop_back();
    }
    return tok;
}

// Converts a register string to a 5-bit binary notation.
string reg_bin(string reg) 
{
    reg = rm_comma(reg);
    if (reg.empty() || reg[0] != 'x') 
    {
        cerr << "ERROR: Invalid register format: " << reg << endl;
        return "00000";
    }
    try {
        int rn = stoi(reg.substr(1));
        return bitset<5>(rn).to_string();
    } catch (...) {
        cerr << "ERROR: Invalid register: " << reg << endl;
        return "00000";
    }
}

// Creates a binary string with the given bit-width from the then value string.
// Represents negative numbers using the two's complement.
string imm_bin(string imm, int bits) {
    try {
        int immVal = stoi(imm);
        unsigned int uVal = static_cast<unsigned int>(immVal) & ((1u << bits) - 1);
        return bitset<32>(uVal).to_string().substr(32 - bits, bits);
    } catch (...) {
        cerr << "ERROR: Invalid immediate: " << imm << endl;
        return string(bits, '0');
    }
}

// Converts an R-type instruction to its binary form in 32 bits.
string enc_r(string ins, string rd, string rs1, string rs2) 
{
    R_t r = instrmap_r[ins];
    return r.f7 + reg_bin(rs2) + reg_bin(rs1) +
           r.f3 + reg_bin(rd) + r.op;
}

// same for U-type instruction.
string enc_u(string ins, string rd, string imm) {
    return imm_bin(imm, 20) + reg_bin(rd) + instrmap_u[ins];
}

//I-type instruction.
string enc_i(string ins, string rd, string rs1, string imm) {
    return imm_bin(imm, 12) + reg_bin(rs1) +
           instrmap_i[ins].f3 + reg_bin(rd) +
           instrmap_i[ins].op;
}

// SB-type instruction therse are branch instructions
string enc_br(string ins, string rs1, string rs2, string imm) {
    B_t br = instrmap_br[ins];
    string immB = imm_bin(imm, 13);
    return string(1, immB[0]) + immB.substr(2, 6) + reg_bin(rs2) +
           reg_bin(rs1) + br.f3 + immB.substr(8, 4) +
           string(1, immB[1]) + br.op;
}
// Encodes a UJ-type instruction.
string enc_uj(string ins, string rd, string imm) {
    int off = stoi(imm);
    uint32_t imm21 = static_cast<uint32_t>(off) & ((1u << 21) - 1);
    uint32_t b20    = (imm21 >> 20) & 0x1;
    uint32_t b10_1  = (imm21 >> 1) & 0x3FF;
    uint32_t b11    = (imm21 >> 11) & 0x1;
    uint32_t b19_12 = (imm21 >> 12) & 0xFF;
    int rn = stoi(rd.substr(1));
    uint32_t mc = (b20<< 31)|(b10_1<< 21) |(b11<< 20) |(b19_12<< 12) |(rn<< 7)|0x6F;
    return bitset<32>(mc).to_string();
}

// S-type instruction-store instructions
string enc_st(string ins, string rs2, string rs1, string imm) {
    string opcd = instrmap_st[ins];
    string immB = imm_bin(imm, 12);
    string imm11_5 = immB.substr(0, 7);
    string imm4_0 = immB.substr(7, 5);
    string f3;
    if (ins == "sb")  f3 = "000";
    if (ins == "sh")  f3 = "001";
    if (ins == "sw")  f3 = "010";
    if (ins == "sd")  f3 = "011";
    return imm11_5 + reg_bin(rs2) + reg_bin(rs1) +
           f3 + imm4_0 + opcd;
}

// Use an enum to differentiate between the assembly file's portions.
enum seg_t { seg_none, seg_txt, seg_dat };

// Map to store label addresses.
unordered_map<string, unsigned int> lbl_map;
// All lines from the input assembly file are stored in a vector.
vector<string> asm_lines;

// Checks if a string represents a numeric value.
bool is_num(const string &s) {
    if(s.empty()) return false;
    size_t i = (s[0]=='-' || s[0]=='+') ? 1 : 0;
    for(; i < s.size(); i++){
       if(!isdigit(s[i])) return false;
    }
    return true;
}

// Removes whitespace from a string's leading and trailing characters.
string trim_str(const string &s) {
    size_t st = s.find_first_not_of(" \t\n\r");
    if(st == string::npos)
        return "";
    size_t en = s.find_last_not_of(" \t\n\r");
    return s.substr(st, en - st + 1);
}
// Returns an unsigned int after parsing a directive value (such as a.byte value).
unsigned int parse_val(const string &tok) {
    string t = trim_str(tok);
    unsigned int val = 0;
    if(t.substr(0,2) == "0x" || t.substr(0,2) == "0X")
       val = stoul(t, nullptr, 16);
    else
       val = stoul(t);
    return val;
}

