/*
    TVM is a virtual machine for LC-3 based operating systems and it is used for educational purposes.
    Copyright (c) under MIT license
    Written by Erick Ahmed, 2022
*/

/* List of all the libraries needed to run tyvm.c */

//#define __UNIX        // uncomment ONLY if compiling for unix-based operative system

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