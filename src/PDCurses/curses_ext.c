#include "curspriv.h"
#include "curses_ext.h"

bool curses_check_key(void)
{
    return PDC_check_key();
}
