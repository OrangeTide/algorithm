/* cout's RPN calculator
 * http://rm-f.net/~cout/rpn/
 * Please email me at pbranna@clemson.edu if you have any comments
*/

#ifndef RPN_CALC_H
#define RPN_CALC_H

enum {
    RPN_OK,

    RPN_STACK_FULL,
    RPN_BAD_PARAM,
    RPN_EMPTY_STACK,
    RPN_NONEMPTY_STACK,
    RPN_ILLEGAL,
    RPN_DIVIDE_BY_ZERO,
    RPN_OUT_OF_MEMORY,

    RPN_UNKNOWN
};

int rpn_calc(const char *input, char *output, size_t out_len);

void rpn_calc_init(void);
void rpn_calc_close(void);

#endif
