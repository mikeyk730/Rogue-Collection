#include "globals.h"

int version_has_arrow_bug()
{
    return version <= RV36B;
}