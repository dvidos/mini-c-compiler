#pragma once


extern int errors_count;

// call this to show error message and signal failure 
void error(const char *filename, int line_no, char *msg, ...);


