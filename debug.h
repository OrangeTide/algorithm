#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG(...)      fprintf(stderr, "DEBUG: " __VA_ARGS__)
#define ERROR(...)      fprintf(stderr, "ERROR: " __VA_ARGS__)
#define TODO(...)       fprintf(stderr, "TODO: " __VA_ARGS__)
#define ERRORP(x)       perror("ERROR: " x)
#define FAIL(...)       do{ fprintf(stderr, "FAILURE: " __VA_ARGS__); exit(EXIT_FAILURE); }while(0)
#define ASSERT(cond)    if(cond) { fprintf(stderr, "ASSERT: " #cond); exit(EXIT_FAILURE); }
#endif /* */
