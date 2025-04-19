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
// On Linux: build with g++ main.cpp -o convert
// or use vscode and c++ extension for your OS

// example input of a space ship 4 blocks by 4 (8 low res pixels by 8)
// ---oo---
// ---oo---
// --o--o--
// -oooooo-
// oooooooo
// oooooooo
// ---oo---
// --------
// this becomes the output that can be directly pasted into the z80 (pasmo assembler) source file
//
//This is the small space ship/plane as a 4x4 character block for the zx81
//
//   DB $00,$85,$05,$00
//   DB $87,$82,$81,$04
//   DB $80,$80,$80,$80
//   DB $00,$02,$01,$00

// example z80 subroutine to draw the sprite
//;;; hl = start of sprite memory
//;;; de = offset position in screen memory top left of sprite - no limit check done (yet)
//;;; c  = width of sprite (normally 8 to keep things "simple")
//;;; b  = rows in sprite (normally 8 to keep things "simple")
//drawSprite
//    push bc
//    push de
//    ld b, 0               ;; just doing columns in c so zero b
//    ldir                  ;; ldir repeats ld (de), (hl) until bc = 0 and increments hl and de
//    pop de
//    ex de, hl
//    ld bc, 33             ;; move next write position to next row
//    add hl, bc
//    ex de, hl
//    pop bc
//    djnz drawSprite
//    ret



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

    std::cout << "Number of rows == " << fileAsStrVec.size() << " number of columns==" << currentLineLen << std::endl;
    std::cout << "This becomes a graphic of " << fileAsStrVec.size() / 2 << "x" << currentLineLen/2 << " character blocks"<< std::endl;
    

    // need to check each 2x2 block for the pixel type, the -1 is to go to penulimate row in outer loop
    // as the nested loop will handle this
    
    for (auto rowOuter = 0; rowOuter < fileAsStrVec.size() - 1; rowOuter+=2)
    {   
        for (auto columnOuter = 0; columnOuter < fileAsStrVec[rowOuter].size() - 1; columnOuter+=2)
        {
            int32_t linearIndex = 0;
            std::string linearVersion = "    ";
            for (auto row = 0; row < 2; row++)
            {
                for (auto col = 0; col < 2; col++)
                {
                    linearVersion[linearIndex++] = fileAsStrVec[row+rowOuter][col+columnOuter];
                }
            }
            bool found = false;

            for (size_t fIndex = 0; fIndex < patterns.size(); fIndex++)
            {
                if (patterns[fIndex].find(linearVersion) != std::string::npos)
                {
                    oStream << patternHexCodes.at(fIndex);

                    if (columnOuter < fileAsStrVec[rowOuter].size() - 2)
                    {
                        oStream << ",";
                    }
                    found = true; 
                } 
            }
            if (found == false)
            {
                std::cerr << "unknown character combination at row" << rowOuter << ", col " << columnOuter << 
                    " inserting blank to maintain row column in output!!" << std::endl;
                    oStream << patternHexCodes.at(0);

                if (columnOuter < fileAsStrVec[rowOuter].size() - 2)
                {
                    oStream << ",";
                }
            }                  
        }

        oStream <<std::endl;
        if (rowOuter < fileAsStrVec.size() - 2)
        {
            oStream << "   DB ";
        }
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