void score(int amount, int flags, char monst);

void get_scores(struct LeaderboardEntry *top10);

void put_scores(struct LeaderboardEntry *top10);

void pr_scores(int newrank, struct LeaderboardEntry *top10);
int add_scores(struct LeaderboardEntry *newscore, struct LeaderboardEntry *oldlist);

//death: Do something really fun when he dies
void death(char monst);

//total_winner: Code for a winner
void total_winner();

//killname: Convert a code to a monster name
char *killname(char monst, bool doart);
