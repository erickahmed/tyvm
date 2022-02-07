/* preprocessor directves needed to run tyvm.c */

#define __UNIX              // used to modify code whether compiling on a Unix-based OS or a Windows machine

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

#ifndef __UNIX
    HANDLE hStdin = INVALID_HANDLE_VALUE;
#endif

#define TRUE 1
#define FALSE 0