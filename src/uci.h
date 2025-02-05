#ifndef UCI_H
#define UCI_H

#include <string>
#include "position.h"

using namespace std;

extern int quit;
extern int movestogo;
extern int movetime;
extern int uci_time;
extern int inc;
extern int starttime;
extern int stoptime;
extern int timeset;
extern int stopped;
extern int numThreads;

// UCI PROTOCOL
int uci_parse_move(thrawn::Position* pos, const char* move_str);

void uci_parse_position(thrawn::Position* pos, const char* command);

void uci_parse_go(thrawn::Position* pos, const char* command);

void uci_loop(thrawn::Position* pos);


// TIME CONTROL 
int input_waiting();

void read_input();

int get_time_ms();

void communicate();

void reset_time_control();

#endif