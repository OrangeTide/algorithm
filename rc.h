#ifndef RC_H
#define RC_H

typedef enum { NODETYPE_NIL, NODETYPE_TREE, NODETYPE_NUMBER, NODETYPE_STRING } nodetype_t; 

struct config_node {
	struct config_node *next;
	struct config_node *child;
	char *name;
	nodetype_t type;
	union {
		long long number;
                char *str;
	} value;
};

struct config_node *config_parser(const char *filename);
struct config_node *config_find(struct config_node *root, const char *name);
void config_free(struct config_node *root);
int config_get_str(struct config_node *item,char *buf,size_t buf_max);
int config_get_int(struct config_node *item,int *i);


#endif /* RC_H */
