#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "greatest/greatest.h"

SUITE_EXTERN(test_static_array);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(test_static_array);
    GREATEST_MAIN_END();        
}


