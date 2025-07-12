#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Function to read a file and return its contents as a string
char* read_file(const char* filename);

// Function to write a string to a file
int write_file(const char* filename, const char* content);

#endif // UTILS_H