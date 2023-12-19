#ifndef EVALUATION_H
#define EVALUATION_H

#include<array>

using namespace std;

// material
extern const std::array<int, 12> material_score;
extern const std::array<int, 12> material_score_eg;

// position
extern const std::array<int, 64> wKnight_mg;
extern const std::array<int, 64> wKnight_eg;
extern const std::array<int, 64> wBishop_mg;
extern const std::array<int, 64> wBishop_eg;
extern const std::array<int, 64> wRook_mg;
extern const std::array<int, 64> wRook_eg;
extern const std::array<int, 64> wQueen_mg;
extern const std::array<int, 64> wQueen_eg;
extern const std::array<int, 64> wKing_mg;
extern const std::array<int, 64> wKing_eg;
extern const std::array<int, 64> wPawn_mg;
extern const std::array<int, 64> wPawn_eg;

extern const std::array<int, 128> mirror_score;

int evaluate();

#endif