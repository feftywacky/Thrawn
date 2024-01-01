#include "misc.h"
#include "bitboard.h"
#include "transposition_table.h"
#include "evaluation.h"
#include "nnue.h"

const std::string version = " v2.0";


void init_all()
{
    // init_magic_nums(); // used to help generate magic bitboards
    
    init_leaping_attacks();
    init_sliding_attacks(bishop);
    init_sliding_attacks(rook);

    // init hashkeys
    init_hashkeys();
    init_hashmap(128); // default of 64 MB

    // init eval masks
    init_eval_masks();

    // init NNUE
    nnue_init("nn-62ef826d1a6d.nnue");
}