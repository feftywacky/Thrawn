#ifndef UCI_H
#define UCI_H

#include <string>

using namespace std;

int uci_parse_move(const char* move_str);

void uci_parse_position(const char* command);

void uci_parse_go(const char* command);

void uci_loop();

#endif