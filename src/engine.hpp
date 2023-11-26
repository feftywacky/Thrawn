#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "bitboard.hpp"

using namespace std;


class Engine
{
public: 

    //constructor
    Engine();


    Bitboard board;

    std::vector<int> moves;

    void generate_moves();
    

private:
        

};

#endif