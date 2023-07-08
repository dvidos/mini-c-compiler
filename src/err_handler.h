#pragma once


extern int warnings_count;
extern int errors_count;

// call these to show messages and signal failure about a file/line
void warn_at(const char *filename, int line_no, char *msg, ...);
void error_at(const char *filename, int line_no, char *msg, ...);

// call these for non-source related issues
void error(char *msg, ...);
void fatal(char *msg, ...);

