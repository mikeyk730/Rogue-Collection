#include "key_utility.h"

bool IsLetterKey(unsigned char keycode)
{
    return (keycode >= 'a' && keycode <= 'z');
}

bool IsDirectionKey(unsigned char keycode)
{
    switch (keycode)
    {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        return true;
    }
    return false;
}
