#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

// code to generate a Sinclair zx81 sprite based on the graphics character blocks in low res mode
// the input file should consist of a number of rows with equal length
// each row will contain either a number of '-' or o representing "low res pixel" on or off
// the "normal grey" pixels are represented by *
// the "inverse grey" pixels are represented by @
// 

// example input of a space ship 4 blocks by 4 (8 low res pixels by 8)
// ---oo---
// ---oo---
// --o--o--
// -oooooo-
// oooooooo
// oooooooo
// ---oo---

// this becomes the output that can be directly pasted into the z80 (pasmo assembler) source file
//
//spriteData
//    DB  $00,$85,$05,$00
//    DB  $00,$06,$86,$00
//    DB  $81,$80,$80,$82
//    DB  $03,$84,$07,$03

// the zx81 graphic blocks are defined as follows 2 by 2
// the first 11 are normal the last 11 are inverse video and are +128 of the normal ones
// symbol   hex   int   |   symbol     hex   int
//====================================================
// --       0     0     |   oo         $80   128
// --                   |   oo         
//====================================================
// o-       1     1     |   -o         $81   129
// --                   |   oo
//====================================================
// -o       2     2     |   o-         $82   130
// --                   |   oo
//====================================================
// oo       3     3     |   --         $83   131
// --                   |   oo
//====================================================
// --       4     4     |   oo         $84   132
// o-                   |   -o
//====================================================
// o-       5     5     |   -o         $85   133
// o-                   |   -o
//====================================================
// -o       6     6     |   o-         $86   134
// o-                   |   -o 
//====================================================
// oo       7     7     |   --         $87   135
// o-                   |   -o         
//====================================================
// **       8     8     |   @@         $88   136
// **                   |   @@
//====================================================
// --       9     9     |   oo         $89   137
// **                   |   @@
//====================================================
// **       A     10    |   @@         $90   138
// --                   |   oo
//====================================================


int32_t parseInput(const std::string & inFileName, const std::string & outFileName)
{
    std::fstream iStream(inFileName, std::ios::in);
    std::fstream oStream(outFileName, std::ios::out);

    if (!iStream.is_open() || !oStream.is_open()) 
    {
       std::cerr << "Error: Could not open the file(s)!" << std::endl;
       return EXIT_FAILURE;
    }
    
    std::string line;
    std::vector<std::string> fileAsStrVec;

    while (std::getline(iStream, line)) 
    {
       fileAsStrVec.push_back(line);
    }
    for (auto& elem:fileAsStrVec)
    {
        for (char ch : elem)
        {
            if (ch == 'o')
            {
                std::cout << "PIXEL ";
            }
            else if (ch == '-') 
            {
                std::cout << "BLANK ";
            } 
            else 
            {
                std::cout << "OTHER ";
            }
        }
        std::cout << std::endl;
    }
    iStream.close();
    oStream.close();
    std::cout << "----" << std::endl;
    return EXIT_SUCCESS;
}

int main(int argc, char * argv[])
{
    int32_t retVal = EXIT_SUCCESS;
    std::string inFile("NOT_SET");
    std::string outFile("NOT_SET");

    std::cout << "Text based ZX81 Sprite Editor by Adrian Pilkington(2025)" << std::endl;
    if (argc == 3)
    {
        inFile = argv[1];
        outFile= argv[2];
        std::cout << "Using input file=" << inFile << " outputting to " << outFile << std::endl;
        parseInput(inFile,outFile);
    }
    else
    {
        std::cout << "useage: " << argv[0] << " <input file> <output file>" << std::endl;
        retVal = EXIT_FAILURE;
    }
    return retVal;
}