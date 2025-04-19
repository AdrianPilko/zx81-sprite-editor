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

// this vecotr stores the pattern to match, and linearly so pattern 3 form table able becomes "oo--"
// the second row are the inverse patterns, 22 in total
const std::vector<std::string> patterns = {"----","o---","-o--","oo--","--o-","o-o-","-oo-","ooo-","****","--**","**--",
                                           "oooo","-ooo","o-oo","--oo","oo-o","-o-o","o--o","---o","@@@@","oo@@","@@oo"};
// these are the codes to match the patterns as will be written to the output file
const std::vector<std::string> patternHexCodes = {"$00","$01","$02","$03","$04","$05","$06","$07","$08","$09","$0A",
                                                  "$80","$81","$82","$83","$84","$85","$86","$87","$88","$89","$90"};

                                            
int32_t parseInput(const std::string & inFileName, const std::string & outFileName)
{
    std::fstream iStream(inFileName, std::ios::in);
    std::fstream oStream(outFileName, std::ios::out);

    if (!iStream.is_open() || !oStream.is_open()) 
    {
       std::cerr << "Error: Could not open the file(s)!" << std::endl;
       return EXIT_FAILURE;
    }

    oStream << "spriteData" << std::endl;
    oStream << "   DB ";   
    std::string line;
    std::vector<std::string> fileAsStrVec;
    int32_t lastLineLen = 0;
    int32_t currentLineLen = 0;
    while (std::getline(iStream, line)) 
    {
        currentLineLen = line.size();
        // check all lines are the same length and have to be even number
        // to maintain the 2x2 block valid size
        if ((currentLineLen != lastLineLen && lastLineLen != 0) || (currentLineLen % 2 != 0))
        {
            std::cout << "ERROR: Lines not all even length" << std::endl;
            return EXIT_FAILURE;
        }
        lastLineLen = currentLineLen;
        fileAsStrVec.push_back(line);
    }
    // the number of lines also has to be even
    if (fileAsStrVec.size() % 2 != 0)
    {
        std::cout << "ERROR: number of lines is not even" << fileAsStrVec.size() << std::endl;
        return EXIT_FAILURE;
    }

    // need to check each 2x2 block for the pixel type, the -1 is to go to penulimate row in outer loop
    // as the nested loop will handle this
    int32_t linearIndex = 0;
    int32_t resultIndex = 0;
    std::string linearVersion = "    ";
    bool found = false;
    
    for (auto row = 0; row < fileAsStrVec.size() - 1; row++)
    {   
        for (auto columnOuter = 0; columnOuter < fileAsStrVec[row].size() - 1; columnOuter++)
        {
            for (auto col = 0; col < 2; col++)
            {
                linearVersion[linearIndex++] = fileAsStrVec[row][col+columnOuter];
            }
            if (linearIndex >= 4)
            {
                linearIndex = 0;
                for (size_t fIndex = 0; fIndex < patterns.size(); fIndex++)
                {
                    if (patterns[fIndex].find(linearVersion) != std::string::npos)
                    {
                        found = true;
                        oStream << patternHexCodes.at(fIndex) << ",";
                    } 
                }
                std::string linearVersion = "    ";
            }
        }

        oStream <<std::endl;
        oStream << "   DB ";
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
        if (parseInput(inFile,outFile) != EXIT_SUCCESS)
        {
            std::cout << "Fault found in input file" << std::endl;
        }
    }
    else
    {
        std::cout << "useage: " << argv[0] << " <input file> <output file>" << std::endl;
        retVal = EXIT_FAILURE;
    }
    return retVal;
}