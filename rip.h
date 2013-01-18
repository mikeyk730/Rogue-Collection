void score(int amount, int flags, char monst);

void get_scores(struct sc_ent *top10);

void put_scores(struct sc_ent *top10);

void pr_scores(int newrank, struct sc_ent *top10);
int add_scores(struct sc_ent *newscore, struct sc_ent *oldlist);

//death: Do something really fun when he dies
void death(char monst);

//total_winner: Code for a winner
void total_winner();

//killname: Convert a code to a monster name
char *killname(char monst, bool doart);
