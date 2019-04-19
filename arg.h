/* See LICENSE file for copyright and license details */
#ifndef ARG_H
#define ARG_H

#define ARG_BEGIN(argv) \
    for (; (argv)[0]; ++(argv)) { \
        if ('-' != (argv)[0][0]) \
            break; \
        (argv)[0] += 1; \
        while ('\0' != (argv)[0][0]) { \
            (argv)[0] += 1; \
            switch ((argv)[0][-1])

#define ARG_END break;}}

/* Terminate the argument list */
#define ARGT(argv) ((argv[1]) = NULL)

/* Retrieve the current flag */
#define ARGF(argv) ((argv)[0][-1])

/* Retrieve the current argument */
#define ARGP(argv) ('\0' == (argv)[0][0] ? (++(argv))[0] : (argv)[0])

#endif
