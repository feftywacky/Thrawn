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
#include <chrono>
#include <unistd.h>
#include <windows.h>
#include <stdio.h>

using namespace std;

/*
TIMING CONTROL AND UCI
UCI PROTOCOL CODE REFERENCES TO VICE CHESS ENGINE BY RICHARD ALBERT
*/

// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int uci_time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

int get_time_ms() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto currentTimeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
    std::chrono::milliseconds duration = currentTimeMs.time_since_epoch();
    return static_cast<int>(duration.count());
}

int input_waiting() {
    static int init = 0;
    static HANDLE inh;
    static DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        int pipe = !GetConsoleMode(inh, &dw);

        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) {
        return dw;
    } 
    else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
    
}

void read_input() {
    // Bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "Listen" to STDIN
    if (input_waiting()) {
        stopped = 1;

        // Loop to read bytes from STDIN
        do {
            bytes = read(fileno(stdin), input, 256);
        }

        // Until bytes are available
        while (bytes < 0);

        // Searches for the first occurrence of '\n'
        endc = strchr(input, '\n');

        // If a newline character is found, set its value at the pointer to 0
        if (endc)
            *endc = 0;

        // If input is available
        if (strlen(input) > 0) {
            // Match UCI "quit" command
            if (!strncmp(input, "quit", 4)) {
                // Tell the engine to terminate execution
                quit = 1;
            }

            // Match UCI "stop" command
            else if (!strncmp(input, "stop", 4)) {
                // Tell the engine to terminate execution
                quit = 1;
            }
        }
    }
}

// a bridge function to interact between search and GUI input
void communicate() {
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptime) {
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}

/*
UCI PROTOCOL
*/

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

    // Infinite search
    if (strstr(command, "infinite") != nullptr) {}

    // Match UCI "binc" command
    if (strstr(command, "binc") != nullptr && colour_to_move == 1) {
        // Parse black time increment
        inc = atoi(strstr(command, "binc") + 5);
    }

    // Match UCI "winc" command
    if (strstr(command, "winc") != nullptr && colour_to_move == 0) {
        // Parse white time increment
        inc = atoi(strstr(command, "winc") + 5);
    }

    // Match UCI "wtime" command
    if (strstr(command, "wtime") != nullptr && colour_to_move == 0) {
        // Parse white time limit
        uci_time = atoi(strstr(command, "wtime") + 6);
    }

    // Match UCI "btime" command
    if (strstr(command, "btime") != nullptr && colour_to_move == 1) {
        // Parse black time limit
        uci_time = atoi(strstr(command, "btime") + 6);
    }

    // Match UCI "movestogo" command
    if (strstr(command, "movestogo") != nullptr) {
        // Parse number of moves to go
        movestogo = atoi(strstr(command, "movestogo") + 10);
    }

    // Match UCI "movetime" command
    if (strstr(command, "movetime") != nullptr) {
        // Parse amount of time allowed to spend to make a move
        movetime = atoi(strstr(command, "movetime") + 9);
    }

    // Match UCI "depth" command
    if (strstr(command, "depth") != nullptr) {
        // Parse search depth
        depth = atoi(strstr(command, "depth") + 6);
    }

    // If move time is available, set time and moves to go
    if (movetime != -1) {
        uci_time = movetime;
        movestogo = 1;
    }

    // Initialize start time
    starttime = get_time_ms();

    // If time control is available
    if (uci_time != -1) {
        // Set the timeset flag
        timeset = 1;

        // Set up timing
        uci_time /= movestogo;
        uci_time -= 50;
        stoptime = starttime + uci_time + inc;
    }

    // If depth is not available, set depth to 64 plies
    if (depth == -1) {
        depth = 64;
    }

    // Print debug info
    std::cout << "time:" << uci_time << " start:" << starttime << " stop:" << stoptime
              << " depth:" << depth << " timeset:" << timeset << std::endl;

    // Search position
    search_position(depth);
}

void uci_loop() {
    // reset stdin and stdout
    std::ios_base::sync_with_stdio(false);
    
    string input;
    
    // print engine info
    cout << "ThrawnEngine v1\n";
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
            cout << "ThrawnEngine v1\n";
            cout << "Feiyu Lin\n";
            cout << "uciok\n";
        }
    }
}


