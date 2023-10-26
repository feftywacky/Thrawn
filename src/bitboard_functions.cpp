#include <cstdint>
#include <vector>
#include <bitset>
#include <iostream>
#include <map>
#include <set>

using namespace std;

uint64_t set_bit(uint64_t bitboard, int bit) {
    return bitboard |= (1ULL << bit);
}

uint64_t clear_bit(uint64_t bitboard, int bit) {
    return bitboard & ~(1ULL << bit);
}

uint64_t get_bit(uint64_t bitboard, int bit){
    return bitboard & (1ULL << bit);
}

bool is_bit_set(uint64_t bitboard, int bit) {
    return (bitboard & (1ULL << bit)) != 0;
}

uint64_t toggle_bit(uint64_t bitboard, int bit) {
    return bitboard ^ (1ULL << bit);
}

vector<int> get_squares_from_bb(uint16_t bitboard)
{
    vector<int> squares;
    bitset<64> bits(bitboard);

    for (int i=0;i<64;i++)
    {
        if (bits[i]==1)
            squares.push_back(i);
    }
    return squares;
}


void pprint_bb(uint64_t bitboard) {
    std::string bitboardStr = "";
    for (int i = 0; i < 64; i++) {
        bitboardStr += (bitboard & (1ULL << i)) ? '1' : '0';
    }
    
    int displayRank = 8;
    std::vector<std::string> board;
    for (int i = 0; i < 64; i += 8) {
        board.push_back(bitboardStr.substr(i, 8));
    }

    for (int i = board.size() - 1; i >= 0; i--) {
        std::cout << displayRank << " ";
        displayRank--;
        for (char square : board[i]) {
            if (square == '1') {
                std::cout << " 1";
            } else {
                std::cout << " 0";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "  ";
    for (char c = 'A'; c < 'A' + 8; c++) {
        std::cout << " " << c;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

void print_bitboard(uint64_t bitboard) {
    std::cout << std::endl;

    // Loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        // Loop over board files
        for (int file = 0; file < 8; file++) {
            // Convert file & rank into square index
            int square = rank * 8 + file;

            // Print ranks
            if (!file)
                std::cout << "  " << 8 - rank << " ";

            // Print bit state (either 1 or 0)
            std::cout << " " << (bitboard & (1ULL << square) ? 1 : 0);
        }

        // Print a new line every rank
        std::cout << std::endl;
    }

    // Print board files
    std::cout << "\n     a b c d e f g h\n\n";

    // Print bitboard as an unsigned decimal number
    std::cout << "     Bitboard as unsigned decimal: " << bitboard << std::endl;
}
