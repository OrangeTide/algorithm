#ifndef DEBUG_H
#define DEBUG_H

#ifdef NDEBUG
#  define DEBUG(...)      /**/
#  define TODO(...)       /**/
#  define ASSERT(cond)    /**/
#else
#  define DEBUG(...)      fprintf(stderr, "DEBUG: " __VA_ARGS__)
#  define TODO(...)       fprintf(stderr, "TODO: " __VA_ARGS__)
#  define ASSERT(cond)    if(cond) { fprintf(stderr, "ASSERT: " #cond); exit(EXIT_FAILURE); }
#endif

#define ERROR(...)      fprintf(stderr, "ERROR: " __VA_ARGS__)
#define ERRORP(x)       perror("ERROR: " x)
#define FAIL(...)       do{ fprintf(stderr, "FAILURE: " __VA_ARGS__); exit(EXIT_FAILURE); }while(0)
#endif /* */

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
