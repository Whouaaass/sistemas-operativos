/**
 * @file strprocessor.c
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @author Joakim Söderberg <https://github.com/JoakimSoderberg>
 * @brief Implementación de manejo personalizado de strings
 *
 * @copyright MIT License
 *
 */

#include "strprocessor.h"

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>
#include <termios.h>

#ifdef _WIN32
#include <shellapi.h>
#include <wchar.h>
#include <windows.h>
#endif

char **split_commandline(const char *cmdline, int *argc) {
    size_t i;
    char **argv = NULL;
    assert(argc);

    if (!cmdline) {
        return NULL;
    }

// Posix.
#ifndef _WIN32
    {
        int ret;
        wordexp_t p;
        memset(&p, 0, sizeof(p));

        // Note! This expands shell variables (might be a security issue).
        if ((ret = wordexp(cmdline, &p, 0))) {
            return NULL;
        }

        *argc = p.we_wordc;

        if (!(argv = calloc(*argc, sizeof(char *)))) {
            goto fail;
        }

        for (i = 0; i < p.we_wordc; i++) {
            if (!(argv[i] = strdup(p.we_wordv[i]))) {
                goto fail;
            }
        }

        // Note that on some OSX versions this does not free all memory (10.9.5)
        wordfree(&p);

        return argv;
    fail:
        p.we_offs = 0;
        wordfree(&p);
    }
#else   // WIN32
    {
        // TODO: __getmainargs is an alternative...
        // https://msdn.microsoft.com/en-us/library/ff770599.aspx
        wchar_t **wargs = NULL;
        size_t needed = 0;
        wchar_t *cmdlinew = NULL;
        size_t len = strlen(cmdline) + 1;

        if (!(cmdlinew = calloc(len, sizeof(wchar_t)))) {
            goto fail;
        }

        if (!MultiByteToWideChar(CP_ACP, 0, cmdline, -1, cmdlinew, len)) {
            goto fail;
        }

        if (!(wargs = CommandLineToArgvW(cmdlinew, argc))) {
            goto fail;
        }

        if (!(argv = calloc(*argc, sizeof(char *)))) {
            goto fail;
        }

        // Convert from wchar_t * to ANSI char *
        for (i = 0; i < *argc; i++) {
            // Get the size needed for the target buffer.
            // CP_ACP = Ansi Codepage.
            needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1, NULL, 0, NULL,
                                         NULL);

            if (!(argv[i] = malloc(needed))) {
                goto fail;
            }

            // Do the conversion.
            needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1, argv[i],
                                         needed, NULL, NULL);
        }

        if (wargs) LocalFree(wargs);
        free(&cmdlinew);
        return argv;

    fail:
        if (wargs) LocalFree(wargs);
        free(&cmdlinew);
    }
#endif  // WIN32

    if (argv) {
        for (i = 0; i < *argc; i++) {
            if (argv[i]) free(argv[i]);
            argv[i] = NULL;
        }

        free(argv);
    }

    return NULL;
}

ssize_t my_getpass(char **lineptr, size_t *n, FILE *stream) {
    struct termios old, new;
    int nread;

    /* Turn echoing off and fail if we can't. */
    if (tcgetattr(fileno(stream), &old) != 0) return -1;
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stream), TCSAFLUSH, &new) != 0) return -1;

    /* Read the password. */
    nread = getline(lineptr, n, stream);

    /* Restore terminal. */
    (void)tcsetattr(fileno(stream), TCSAFLUSH, &old);

    return nread;
}