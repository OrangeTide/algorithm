#ifndef PROTO_H
#define PROTO_H
int proto_init(void);
void proto_shutdown(void);
int proto_result(char *dest, size_t max, const char *in);
#endif

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
