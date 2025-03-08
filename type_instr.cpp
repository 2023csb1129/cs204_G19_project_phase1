#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <vector>
#include <cstdint> // ✅ Fix for uint8_t, uint16_t, etc.

using namespace std;

// Define memory space (fixed size)
vector<uint8_t> memory(0x10000000 + 0x1000, 0); // Simulating memory space
unordered_map<string, int> directiveSize = {
    {".byte", 1}, {".half", 2}, {".word", 4}, {".dword", 8}};

int dataAddress = 0x10000000; // Starting address for .data section

// Function to handle assembler directives
void handleDirective(const vector<string> &tokens)
{
    if (tokens.size() < 3)
    {
        cerr << "ERROR: Invalid directive format." << endl;
        return;
    }

    string directive = tokens[0];
    string value = tokens[2];

    if (directive == ".asciz")
    {
        // Extract string inside quotes
        size_t start = value.find('"');
        size_t end = value.rfind('"');
        if (start == string::npos || end == string::npos || start == end)
        {
            cerr << "ERROR: Invalid .asciz string format!" << endl;
            return;
        }

        string strValue = value.substr(start + 1, end - start - 1);

        // Store each character in memory
        for (char c : strValue)
        {
            memory[dataAddress++] = static_cast<uint8_t>(c);
        }
        memory[dataAddress++] = 0; // Null terminator
    }
    else if (directiveSize.count(directive))
    { // ✅ Handle `.byte`, `.half`, `.word`, `.dword`
        int size = directiveSize[directive];

        // Align memory address properly
        while (dataAddress % size != 0)
        {
            dataAddress++;
        }

        uint64_t dataValue = stoull(value);

        // Store the value in little-endian order
        for (int i = 0; i < size; i++)
        {
            memory[dataAddress + i] = (dataValue >> (8 * i)) & 0xFF;
        }

        dataAddress += size;
    }
}

// Function to print stored memory
void printMemory()
{
    cout << "Stored Memory:" << endl;
    for (size_t i = 0x10000000; i < dataAddress; i++)
    {
        printf("0x%08X → %02X\n", (uint32_t)i, memory[i]);
    }
}

// Main function
int main()
{
    vector<vector<string>> testInputs = {
        {".byte", "data1", "65"},        // ASCII 'A'
        {".half", "data2", "300"},       // 0x012C
        {".word", "data3", "1000"},      // 0x000003E8
        {".dword", "data4", "5000"},     // 0x0000000000001388
        {".asciz", "data5", "\"Hello\""} // "Hello\0"
    };

    for (const auto &tokens : testInputs)
    {
        handleDirective(tokens);
    }

    printMemory();
    return 0;
}
