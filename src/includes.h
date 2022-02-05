/* List of all the libraries needed to run tyvm.c */

/* universal libraries */
    #include <stdint.h>
    #include <stdio.h>
    #include <signal.h>

/* unix only libraries */
#ifdef __UNIX
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>

    /* sys libraries */
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/termios.h>
    #include <sys/mman.h>
/* windows only libraries */
#else
    #include <Windows.h>
    #include <conio.h>
#endif