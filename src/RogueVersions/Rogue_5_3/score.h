/*
 * Score file structure
 *
 * @(#)score.h	4.3 (NMT from Berkeley 5.2) 8/25/83
 */

struct sc_ent {
    char sc_name[MAXSTR];
    unsigned int sc_flags;
    unsigned int sc_uid;
    unsigned short sc_monster;
    unsigned short sc_score;
    unsigned short sc_level;
};

typedef struct sc_ent SCORE;
