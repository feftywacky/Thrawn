#include "uci.h"
#include "engine.h"
#include "move_helpers.h"
#include "bitboard_helpers.h"
#include "fen.h"
#include "search.h"
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <string>

using namespace std;

int uci_parse_move(const char *move_str)
{
    std::vector<int> moves = generate_moves();
    
    int source = (move_str[0] - 'a') + (8-(move_str[1]- '0')) * 8;
    int target = (move_str[2] - 'a') + (8-(move_str[3]- '0')) * 8;
    int promoted_piece = 0;

    for (int move : moves)
    {
        if (source == get_move_source(move) && target == get_move_target(move))
        {
            promoted_piece = get_promoted_piece(move);

            if (promoted_piece)
            {
                if ((promoted_piece == Q || promoted_piece == q) && move_str[4] == 'q')
                    return move;
                else if ((promoted_piece == R || promoted_piece == r) && promoted_piece == move_str[4] == 'r')
                    return move;
                else if ((promoted_piece == N || promoted_piece == n) && promoted_piece == move_str[4] == 'b')
                    return move;
                else if ((promoted_piece == B || promoted_piece == b) && promoted_piece == move_str[4] == 'n')
                    return move;
                continue;
            }

            // legal move
            return move;
        }
    }

    // illegal move ie. puts king in check
    return 0;
}

void uci_parse_position(const char *command) {
    // Create a non-const pointer for manipulation
    cout<<command<<endl;
    const char *non_const_command = command;

    non_const_command += 9; // shift index to skip 'position' in command

    const char *curr_ch = non_const_command;

    // parse 'startpos' command
    
    if (strncmp(non_const_command, "startpos", 8) == 0)
        parse_fen(start_position);

    // parse 'fen' command
    else 
    {
        curr_ch = strstr(non_const_command, "fen");

        // no 'fen' command found
        if (curr_ch == nullptr)
            parse_fen(start_position);

        else {
            // shift index to next token
            curr_ch += 4;
            parse_fen(curr_ch);
        }
    }

    // parse moves after position is set up
    curr_ch = strstr(non_const_command, "moves");
    
    // if moves are found
    if (curr_ch != nullptr) {

        curr_ch += 6;

        while (*curr_ch) {
            int move = uci_parse_move(curr_ch);

            if (move == 0)
                break;

            make_move(move, all_moves);

            // Move index to the end of the current move
            while (*curr_ch && *curr_ch != ' ')
                curr_ch++;

            // go to next move
            curr_ch++;
        }

    }

    print_board(colour_to_move);

}

void uci_parse_go(const char* command)
{
    int depth = -1;

    char *curr_depth = nullptr;

    // fixed depth search
    if (curr_depth = strstr(command, "depth"))
        depth = atoi(curr_depth+6);
    else
        depth = 6;
    
    // negamax alpha beta pruning search
    cout<<"search depth: "<<depth<<endl;
    search_position(depth);
}

void uci_loop() {
    // reset stdin and stdout
    std::ios_base::sync_with_stdio(false);
    
    string input;
    
    // print engine info
    cout << "fefty engine v1\n";
    cout << "Feiyu Lin\n";
    cout << "uciok\n";
    
    // main loop
    while (true) {

        // reset user/GUI input & output
        input.clear();
        std::cout.flush();
        
        // get user/GUI input
        if (!std::getline(std::cin, input))
            continue;
        
        if (input.empty())
            continue;

        // parse UCI "isready" command
        if (input.compare(0, 7, "isready") == 0) 
        {
            cout << "readyok\n";
            continue;
        }   

        // parse UCI "position" command
        else if (input.compare(0, 8, "position") == 0)
            uci_parse_position(input.c_str());
        
        // parse UCI "ucinewgame" command
        else if (input.compare(0, 10, "ucinewgame") == 0) 
            uci_parse_position("position startpos");
        
        // parse UCI "go" command
        else if (input.compare(0, 2, "go") == 0)
            uci_parse_go(input.c_str());
        
        // parse UCI "quit" command
        else if (input.compare(0, 4, "quit") == 0)
            break;
        
        // parse UCI "uci" command
        else if (input.compare(0, 3, "uci") == 0)
        {
            cout << "fefty engine v1\n";
            cout << "Feiyu Lin\n";
            cout << "uciok\n";
        }
    }
}


