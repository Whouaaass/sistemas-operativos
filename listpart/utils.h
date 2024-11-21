/**
 * @file utils.h
 * @author Fredy Anaya <fredyanaya@unicauca.edu.co>
 * @brief Utilities for the project
 * @copyright MIT License
 */
#ifndef UTILS_H
#define UTILS_H
/**
 * @brief Compares two strings ignoring case
 * @param str_1 First string
 * @param str_2 Second string
 * @return int 0 if the strings are equal, otherwise the difference between the first different character
 */
int strcicmp(char const *str_1, char const *str_2);

#endif