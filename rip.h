score(int amount, int flags, char monst);

get_scores(struct sc_ent *top10);

put_scores(struct sc_ent *top10);

pr_scores(int newrank, struct sc_ent *top10);
add_scores(struct sc_ent *newscore, struct sc_ent *oldlist);

//death: Do something really fun when he dies
death(char monst);

//total_winner: Code for a winner
total_winner();

//killname: Convert a code to a monster name
char *killname(char monst, bool doart);
