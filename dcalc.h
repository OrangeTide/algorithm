/* contributed by demoncrat@efnet.#c on march 2001. */
/* edited by megaton. added double precision numbers */

enum { 
  CODESIZE = 1000,
  STACKSIZE = 1000
};

typedef double Value;

/* Set *result to the value of `expression'.
   Return an error message string, or NULL if there was no error. */
const char *dcalc (Value *result, const char *expression);


