/**
 * @file utils.c
 * @author Fredy Anaya <fredyanaya@unicauca.edu.co>
 * @brief Utilities for the project
 * @copyright MIT License
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

int strcicmp(char const *str_1, char const *str_2)
{
	while (1)
	{
		int d = tolower(*str_1) - tolower(*str_2); 
		if (d != 0 || *str_1 == '\0' || *str_2 == '\0')
			return d;
		str_1++;
		str_2++;
	}
}