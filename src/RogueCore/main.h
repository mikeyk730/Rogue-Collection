#include <memory>

struct DisplayInterface;
struct InputInterface;

//do_quit: Have player make certain, then exit.
bool do_quit();

//leave: Leave quickly, but courteously
void leave();

//fatal: exit with a message
void fatal(char *msg, ...);

//game_main: The main program, of course
int game_main(int argc, char **argv, std::shared_ptr<DisplayInterface> output, std::shared_ptr<InputInterface> input);
