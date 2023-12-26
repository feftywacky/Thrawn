#ifndef UCI_H
#define UCI_H

#include <string>

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

// UCI PROTOCOL
int uci_parse_move(const char* move_str);

void uci_parse_position(const char* command);

void uci_parse_go(const char* command);

void uci_loop();


// TIME CONTROL 
int input_waiting();

void read_input();

int get_time_ms();

void communicate();

void reset_time_control();

#endif