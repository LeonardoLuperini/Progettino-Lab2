#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...) ;
#endif

#endif
