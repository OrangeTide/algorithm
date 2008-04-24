#ifndef NOTIFY_H
#define NOTIFY_H
struct message;
int notify_register(const char *type, void (*func)(void *p, struct message *msg), void *p);
int notify_unregister(const char *type, void (*func)(void *p, struct message *msg));
void notify_report_message(struct message *msg);
#endif

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
