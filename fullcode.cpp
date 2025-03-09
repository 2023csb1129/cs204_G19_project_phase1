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

int main() {
    // Open the input assembly file.
    ifstream in("input.asm");
    if (!in) {
        cerr << "Error: Unable to open input.asm" << endl;
        return 1;
    }
    
// Read each line from the assembly file and store them in the asm_lines vector.
    string cl;
    while(getline(in, cl)) {
        asm_lines.push_back(cl);
    }
    in.close();
// First,calculate addresses and construct the label map.
 //  To find the location of each label, we read each line.
    seg_t cur_seg = seg_none;
    unsigned int cur_addr = 0;
    for(auto &ln : asm_lines) {
        if(ln.empty() || ln[0]=='#') continue;
        istringstream ls(ln);
        string tok;
        ls >> tok;
        istringstream *p_ls = &ls;
        string extra;
        // Check if the line starts with a label.
        if(tok.back() == ':'){
            string lbl = tok.substr(0, tok.size()-1);
            lbl_map[lbl] = cur_addr; // Save the address of the label.
            getline(ls, extra);
            if(extra.empty())
                continue;
            p_ls = new istringstream(extra);
            (*p_ls) >> tok;
        }
// Set the segment type and base addresses.
        if(tok == ".text") {
            cur_seg = seg_txt;
            cur_addr = 0;
        } else if(tok == ".data") {
            cur_seg = seg_dat;
            cur_addr = 0x10000000;
        } else if(cur_seg == seg_txt) {
            cur_addr += 4; // Each instruction is 4 bytes.
        } else if(cur_seg == seg_dat) {
            // data----add the size of each directive we are taking.
            if(tok == ".byte") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(!v.empty())
                        cur_addr += 1;
                }
            } else if(tok == ".half") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(!v.empty())
                        cur_addr += 2;
                }
            } else if(tok == ".word") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(!v.empty())
                        cur_addr += 4;
                }
            } else if(tok == ".dword") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(!v.empty())
                        cur_addr += 8;
                }
            } else if(tok == ".asciz") {
                string str;
                getline(*p_ls, str);
                size_t st = str.find('\"');
                size_t en = str.rfind('\"');
                if(st != string::npos && en != string::npos && en > st) {
                    string content = str.substr(st+1, en - st - 1);
                    cur_addr += content.size() + 1;
                }
            }
        }
        if(p_ls != &ls)
            delete p_ls;
    }
// Second work iswe use the assembly to create the machine code.
//Each line is processed by this loop, which converts data and instructions into machine code.   
    ofstream out("output.mc");
    if(!out) {
        cerr << "Error: Unable to open output.mc for writing" << endl;
        return 1;
    }
    cur_seg = seg_none;
    unsigned int txt_base = 0;
    unsigned int dat_base = 0x10000000;
    unsigned int cur_txt = 0;
    unsigned int cur_dat = dat_base;
    bool txt_done = false;
    vector<string> txt_lines;
    vector<string> dat_lines;
    
    for(auto &ln : asm_lines) {
        if(ln.empty() || ln[0]=='#') continue;
        istringstream ls(ln);
        string tok;
        ls >> tok;
        istringstream *p_ls = &ls;
        string extra;
        // If the line starts with a label, adjust the stream.
        if(tok.back() == ':'){
            getline(ls, extra);
            if(extra.empty())
                continue;
            p_ls = new istringstream(extra);
            (*p_ls) >> tok;
        }
        // Switch segment if there is a need.
        if(tok == ".text") {
            cur_seg = seg_txt;
            cur_txt = 0;
            continue;
        } else if(tok == ".data") {
            if(cur_seg == seg_txt && !txt_done) {
                ostringstream oss;
                oss << "0x" << hex << cur_txt << " 0xFFFFFFFF , END";
                txt_lines.push_back(oss.str());
                txt_done = true;
            }
            cur_seg = seg_dat;
            cur_dat = dat_base;
            continue;
        }
        // Process instructions in the text segment.
        if(cur_seg == seg_txt) {
            string mc;
            string bin_cmt;
            if(instrmap_r.count(tok)) {
                string rd, rs1, rs2;
                (*p_ls) >> rd >> rs1 >> rs2;
                mc = enc_r(tok, rm_comma(rd), rm_comma(rs1), rm_comma(rs2));
                R_t r = instrmap_r[tok];
                bin_cmt = r.op + "-" + r.f3 + "-" + r.f7 + "-" +
                         reg_bin(rm_comma(rd)) + "-" +
                         reg_bin(rm_comma(rs1)) + "-" +
                         reg_bin(rm_comma(rs2)) + "-NULL";
            } else if(instrmap_i.count(tok)) {
                string rd, opr;
                (*p_ls) >> rd >> opr;
                string imm, rs1;
                size_t pos = opr.find('(');
                if(pos != string::npos) {
                    imm = opr.substr(0, pos);
                    rs1 = opr.substr(pos+1, opr.find(')') - pos - 1);
                } else {
                    rs1 = opr;
                    (*p_ls) >> imm;
                }
                mc = enc_i(tok, rm_comma(rd), rm_comma(rs1), rm_comma(imm));
                I_t it = instrmap_i[tok];
                string immB = imm_bin(rm_comma(imm), 12);
                bin_cmt = it.op + "-" + it.f3 + "-NULL-" +
                         reg_bin(rm_comma(rd)) + "-" +
                         reg_bin(rm_comma(rs1)) + "-" + immB;
            } else if(instrmap_st.count(tok)) {
                string rs2, opr;
                (*p_ls) >> rs2 >> opr;
                string imm, rs1;
                size_t pos = opr.find('(');
                if(pos != string::npos) {
                    imm = opr.substr(0, pos);
                    rs1 = opr.substr(pos+1, opr.find(')') - pos - 1);
                }
                mc = enc_st(tok, rm_comma(rs2), rm_comma(rs1), rm_comma(imm));
                string immB = imm_bin(rm_comma(imm), 12);
                string f3;
                if(tok == "sb")  f3 = "000";
                if(tok == "sh")  f3 = "001";
                if(tok == "sw")  f3 = "010";
                if(tok == "sd")  f3 = "011";
                bin_cmt = instrmap_st[tok] + "-" + f3 + "-NULL-NULL-" +
                         reg_bin(rm_comma(rs1)) + "-" +
                         reg_bin(rm_comma(rs2)) + "-" + immB;
            } else if(instrmap_br.count(tok)) {
                string rs1, rs2, imm;
                (*p_ls) >> rs1 >> rs2 >> imm;
                if(!is_num(imm)) {
                    int off = lbl_map[imm] - (cur_txt+4);
                    imm = to_string(off);
                }
                mc = enc_br(tok, rm_comma(rs1), rm_comma(rs2), rm_comma(imm));
                string immB = imm_bin(rm_comma(imm), 13);
                B_t br = instrmap_br[tok];
                bin_cmt = br.op + "-" + br.f3 + "-NULL-NULL-" +
                         reg_bin(rm_comma(rs1)) + "-" +
                         reg_bin(rm_comma(rs2)) + "-" + immB;
            } else if(instrmap_u.count(tok)) {
                string rd, imm;
                (*p_ls) >> rd >> imm;
                mc = enc_u(tok, rm_comma(rd), rm_comma(imm));
                string immB = imm_bin(rm_comma(imm), 20);
                bin_cmt = instrmap_u[tok] + "-NULL-NULL-" +
                         reg_bin(rm_comma(rd)) + "-NULL-" + immB;
            } else if(instrmap_uj.count(tok)) {
                string rd, lbl;
                ls >> rd >> lbl;
                int off = lbl_map[lbl] - cur_txt;
                string imm = to_string(off);
                mc = enc_uj(tok, rm_comma(rd), rm_comma(imm));
                string immB = imm_bin(rm_comma(imm), 21);
                bin_cmt = instrmap_uj[tok] + "-NULL-NULL-" +
                         reg_bin(rm_comma(rd)) + "-NULL-" + immB;
            } else {
                if(p_ls != &ls)
                    delete p_ls;
                continue;
            }
            ostringstream oss;
            oss << "0x" << hex << cur_txt << " 0x" 
                << setw(8) << setfill('0') << stoul(mc, nullptr, 2)
                << " , " << ln << " # " << bin_cmt;
            txt_lines.push_back(oss.str());
            cur_txt += 4;
        } else if(cur_seg == seg_dat) {
            // now after it is done,process data directives and convert them into machine code lines.
            if(tok == ".byte") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(v.empty()) continue;
                    unsigned int bval = parse_val(v);
                    ostringstream oss;
                    oss << "0x" << hex << cur_dat << " 0x" 
                        << setw(2) << setfill('0') << bval;
                    dat_lines.push_back(oss.str());
                    cur_dat += 1;
                }
            } else if(tok == ".half") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(v.empty()) continue;
                    unsigned int hval = parse_val(v);
                    ostringstream oss;
                    oss << "0x" << hex << cur_dat << " 0x" 
                        << setw(4) << setfill('0') << hval;
                    dat_lines.push_back(oss.str());
                    cur_dat += 2;
                }
            } else if(tok == ".word") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(v.empty()) continue;
                    unsigned int wval = parse_val(v);
                    ostringstream oss;
                    oss << "0x" << hex << cur_dat << " 0x" 
                        << setw(8) << setfill('0') << wval;
                    dat_lines.push_back(oss.str());
                    cur_dat += 4;
                }
            } else if(tok == ".dword") {
                string vals;
                getline(*p_ls, vals);
                istringstream vs(vals);
                string v;
                while(getline(vs, v, ',')) {
                    if(v.empty()) continue;
                    unsigned long long dval = stoull(v, nullptr, 0);
                    ostringstream oss;
                    oss << "0x" << hex << cur_dat << " 0x" 
                        << setw(16) << setfill('0') << dval;
                    dat_lines.push_back(oss.str());
                    cur_dat += 8;
                }
            } else if(tok == ".asciz") {
                string str;
                getline(*p_ls, str);
                size_t st = str.find('\"');
                size_t en = str.rfind('\"');
                if(st != string::npos && en != string::npos && en > st) {
                    string content = str.substr(st+1, en - st - 1);
                    for (char c : content) {
                        ostringstream oss;
                        oss << "0x" << hex << cur_dat << " 0x" 
                            << setw(2) << setfill('0') << (int)c;
                        dat_lines.push_back(oss.str());
                        cur_dat += 1;
                    }
                    ostringstream oss;
                    oss << "0x" << hex << cur_dat << " 0x00";
                    dat_lines.push_back(oss.str());
                    cur_dat += 1;
                }
            }
        }
        if(p_ls != &ls)
            delete p_ls;
    }
    
    // Write the text segment lines to the output file.
    for(auto &l : txt_lines) {
        out << l << "\n";
    }
    out << "\n";
    // Write the data segment lines to the output file.
    for(auto &l : dat_lines) {
        out << l << "\n";
    }
    out.close();
    
    cout << "Successfully converted assembly to machine code in output.mc!" << endl;
    return 0;
}