#include "bitboard.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <bitset>


using namespace std;

// contructor
Bitboard::Bitboard()
{
    init();
}

void Bitboard::init()
{
    // white
    whitePawns = 0xFF00;
    whiteRooks = 0x81;
    whiteKnights = 0x42;
    whiteBishops = 0x24;
    whiteQueens = 0x08;
    whiteKings = 0x10;

    //black
    blackPawns = 0xFF000000000000;
    blackRooks = 0x8100000000000000;
    blackKnights = 0x4200000000000000;
    blackBishops = 0x2400000000000000;
    blackQueens = 0x0800000000000000;
    blackKings = 0x1000000000000000;
}

void Bitboard::printBB()
{
    // Print the chessboard state with a simplified representation
    // For each square, indicate the presence of a piece with 'P', 'R', 'N', 'B', 'Q', 'K', or '-'
    // 'P' represents a white pawn, 'p' represents a black pawn, and so on
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            uint64_t mask = 1ULL << (rank * 8 + file);
            
            char piece = '-';
            
            if (whitePawns & mask) piece = 'P';
            if (whiteRooks & mask) piece = 'R';
            if (whiteKnights & mask) piece = 'N';
            if (whiteBishops & mask) piece = 'B';
            if (whiteQueens & mask) piece = 'Q';
            if (whiteKings & mask) piece = 'K';
            if (blackPawns & mask) piece = 'p';
            if (blackRooks & mask) piece = 'r';
            if (blackKnights & mask) piece = 'n';
            if (blackBishops & mask) piece = 'b';
            if (blackQueens & mask) piece = 'q';
            if (blackKings & mask) piece = 'k';

            std::cout << piece << ' ';
        }
        std::cout << std::endl;
    }
}

void Bitboard::pprint_bb(uint64_t bitboard, int board_size = 8) {
    std::string bitboardStr = "";
    for (int i = 0; i < 64; i++) {
        bitboardStr += (bitboard & (1ULL << i)) ? '1' : '0';
    }
    
    int displayRank = board_size;
    std::vector<std::string> board;
    for (int i = 0; i < 64; i += board_size) {
        board.push_back(bitboardStr.substr(i, board_size));
    }

    for (int i = board.size() - 1; i >= 0; i--) {
        std::cout << displayRank << " ";
        displayRank--;
        for (char square : board[i]) {
            if (square == '1') {
                std::cout << " -";
            } else {
                std::cout << " -";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "  ";
    for (char c = 'A'; c < 'A' + board_size; c++) {
        std::cout << " " << c;
    }
    std::cout << std::endl;
}

void Bitboard::pprint_pieces(std::map<char, std::set<int>> piece_map, int board_size = 8) {
    std::vector<char> board(64, '-');
    for (const auto& entry : piece_map) {
        char piece = entry.first;
        const std::set<int>& squares = entry.second;
        for (int square : squares) {
            board[square] = piece;
        }
    }

    int displayRank = board_size;
    for (int i = board.size() - 1; i >= 0; i -= board_size) {
        std::cout << displayRank << " ";
        displayRank--;
        for (int j = 0; j < board_size; j++) {
            std::cout << " " << board[i - j];
        }
        std::cout << std::endl;
    }

    std::cout << "  ";
    for (char c = 'A'; c < 'A' + board_size; c++) {
        std::cout << " " << c;
    }
    std::cout << std::endl;
}

vector<int> Bitboard::get_squares_from_bb(uint16_t bitboard)
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


uint64_t Bitboard::get_white_pieces()
{
    return whitePawns | whiteRooks | whiteKnights | whiteBishops | whiteQueens | whiteKings;
}

uint64_t Bitboard::get_black_pieces()
{
    return blackPawns | blackRooks | blackKnights | blackBishops | blackQueens | blackKings;
}




