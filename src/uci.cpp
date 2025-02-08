#include "uci.h"
#include "move_generator.h"
#include "move_helpers.h"
#include "bitboard_helpers.h"
#include "transposition_table.h"
#include "bitboard.h"
#include "fen.h"
#include "perft.h"
#include "search.h"
#include "misc.h"
#include "globals.h"
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <string>
#include <chrono>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <sys/ioctl.h>
#endif

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

// Number of threads use for search
int numThreads = 4;

/*
TIME CONTROL
*/
int get_time_ms() {
    return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int input_waiting() {
#ifdef _WIN32
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
#else
    int bytesAvailable;
    ioctl(fileno(stdin), FIONREAD, &bytesAvailable);
    return bytesAvailable;
#endif
}

void read_input() {
    // Bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "Listen" to STDIN
    if (input_waiting()) {
        // cout<<"input waiting"<<"\n";
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
         // cout<<"communicate set stopped = 1"<<"\n";
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}

/*
UCI PROTOCOL
*/

int uci_parse_move(thrawn::Position* pos, const char *move_str)
{
    std::vector<int> moves = generate_moves(pos);
    
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
                else if ((promoted_piece == R || promoted_piece == r) && move_str[4] == 'r')
                    return move;
                else if ((promoted_piece == N || promoted_piece == n) && move_str[4] == 'b')
                    return move;
                else if ((promoted_piece == B || promoted_piece == b) && move_str[4] == 'n')
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

void uci_parse_position(thrawn::Position* pos, const char *command) {
    // Create a non-const pointer for manipulation
    cout<<command<<endl;
    const char *non_const_command = command;

    non_const_command += 9; // shift index to skip 'position' in command

    const char *curr_ch = non_const_command;

    // parse 'startpos' command
    
    if (strncmp(non_const_command, "startpos", 8) == 0)
        parse_fen(pos, start_position);

    // parse 'fen' command
    else 
    {
        curr_ch = strstr(non_const_command, "fen");

        // no 'fen' command found
        if (curr_ch == nullptr)
            parse_fen(pos, start_position);

        else {
            // shift index to next token
            curr_ch += 4;
            parse_fen(pos, curr_ch);
        }
    }

    // parse moves after position is set up
    curr_ch = strstr(non_const_command, "moves");
    
    // if moves are found
    if (curr_ch != nullptr) {

        curr_ch += 6;

        while (*curr_ch) {
            int move = uci_parse_move(pos, curr_ch);

            if (move == 0)
                break;

            pos->repetition_index++;
            pos->repetition_table[pos->repetition_index] = pos->zobristKey;

            make_move(pos, move, all_moves,-1);

            // Move index to the end of the current move
            while (*curr_ch && *curr_ch != ' ')
                curr_ch++;

            // go to next move
            curr_ch++;
        }

    }

    // for debug
    // print_board(colour_to_move);

}

void uci_parse_go(thrawn::Position* pos, const char* command)
{
    reset_time_control();
    int depth = -1;

    // Infinite search
    if (strstr(command, "infinite") != nullptr) {}

    // Match UCI "binc" command
    if (strstr(command, "binc") != nullptr && pos->colour_to_move == 1) {
        // Parse black time increment
        inc = atoi(strstr(command, "binc") + 5);
    }

    // Match UCI "winc" command
    if (strstr(command, "winc") != nullptr && pos->colour_to_move == 0) {
        // Parse white time increment
        inc = atoi(strstr(command, "winc") + 5);
    }

    // Match UCI "wtime" command
    if (strstr(command, "wtime") != nullptr && pos->colour_to_move == 0) {
        // Parse white time limit
        uci_time = atoi(strstr(command, "wtime") + 6);
    }

    // Match UCI "btime" command
    if (strstr(command, "btime") != nullptr && pos->colour_to_move == 1) {
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
    
    depth = depth;

    // If time control is available
    if (uci_time != -1) {
        // Set the timeset flag
        timeset = 1;

        // Set up timing
        uci_time /= movestogo;
        uci_time -= 50; // lag compensation 

        if (uci_time < 0)
        {
            uci_time = 0;
            inc -= 50;
            if (inc<0) 
                inc = 1;
        }
        stoptime = starttime + uci_time + inc;
    }

    // If depth is not available, set depth to 64 plies
    if (depth == -1) {
        depth = 64;
    }

    // Print debug info
    std::cout << "time:" << uci_time << " start:" << static_cast<unsigned int>(starttime) << " stop:" << static_cast<unsigned int>(stoptime)
              << " depth:" << depth << " timeset:" << timeset << std::endl;


    search_position_threaded(pos, depth, numThreads);  
}

void uci_loop(thrawn::Position* pos)
{
    // just make it big enough
    #define INPUT_BUFFER 20000
    
    int max_hashmap_size = 1024; // 1GB
    int mb = 256; // default 256 MB

    // reset STDIN & STDOUT buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    // define user / GUI input buffer
    char input[INPUT_BUFFER];

    while (true)
    {
        // reset user /GUI input
        memset(input, 0, sizeof(input));
        
        // make sure output reaches the GUI
        fflush(stdout);
        
        // get user / GUI input
        if (!fgets(input, INPUT_BUFFER, stdin))
            continue;
        
        // make sure input is available
        if (input[0] == '\n')
            continue;
        
        // parse UCI "isready" command
        if (strncmp(input, "isready", 7) == 0)
        {
            std::cout << "readyok\n";
            continue;
        }
        
        // parse UCI "position" command
        else if (strncmp(input, "position", 8) == 0)
        {
            tt->reset();
            uci_parse_position(pos, input);
        }

        // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            tt->reset();
            uci_parse_position(pos, "position startpos");
        }
        // parse UCI "go" command
        else if (strncmp(input, "go", 2) == 0)
            uci_parse_go(pos, input);
        
        // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0)
            break;
        
        // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0)
        {
            // print engine info
            cout << "id name Thrawn"<< version << "\n";
            cout << "id author Feiyu Lin\n";
            cout << "option name Hash type spin default 256 min 4 max 1024" << max_hashmap_size << "\n";
            cout << "option name Threads type spin default 4 min 1 max 16" << "\n";
            cout << "uciok\n";
        }
        
        else if (!strncmp(input, "setoption name Hash value ", 26)) {			
            // init MB
            sscanf(input,"%*s %*s %*s %*s %d", &mb);
            
            // adjust MB if going beyond the aloowed bounds
            if(mb < 4) mb = 4;
            if(mb > max_hashmap_size) mb = max_hashmap_size;
            
            // set hash table size in MB
            std::cout << "    Set hash table size to " << mb << "MB\n";
            tt->initTable(mb);
        }

        else if (!strncmp(input, "setoption name Threads value ", 29)) {
            int t = 1;
            sscanf(input, "%*s %*s %*s %*s %d", &t);
            if (t < 1) t = 1;
            if (t > 16) t = 16;
            numThreads = t;
            std::cout << "info string Set threads = " << numThreads << std::endl;
        }

        else if (strncmp(input, "perft", 5) == 0)
        {
            perft_run_unit_tests();
        }
    }
}

void reset_time_control()
{
    quit = 0;
    movestogo = 30;
    movetime = -1;
    uci_time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}
