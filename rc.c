/* rc.c : database / configfile reader */
/* CONTEXT-FREE PARSER */

/* TODO:
 * - make use of the current_operation string for better diagnostic output
 * - support the following datatypes: numbers, strings, arrays/lists, flags 
 * - move loop out of parser into expr()
 * - support C style comments
 * - use buf_max in ident() function
 * - don't use = for groups;  use = for lists
 * - allow group names to be followed by a second string
 * - allow 1-word strings without "" around them
 */

/* DONE: 
 * - basic parse syntax
 * - C++ style comments
 * - make ; optional at the end of a group or list or assignment
 * - make = optional
 * 
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include "rc.h"
#include "debug.h"

#define IDENT_MAX    16384 /* buffer used for idents */
#define STRING_MAX    65536 /* buffer used to hold a string */

#define MUST(x)        { if((x)<=0) return 0; }
#define OPTIONAL(x)    { if((x)==EOF) return EOF; }
/* debug versions
#define MUST(x)        { DEBUG("MUST: " #x "\n"); if((x)<=0) return 0; }
#define OPTIONAL(x)    { DEBUG("OPTIONAL: " #x "\n"); if((x)==EOF) return EOF; }
*/

static char current_operation[64] = "doing nothing"; 

static void reset_current_operation(void)
{
    strcpy(current_operation,"doing nothing");
}

static int isident_first(char ch)
{
    return isalpha(ch) || ch=='_';
}

static int isident(char ch)
{
    return isalnum(ch) || ch=='_';
}


static struct config_node *createnode(const char *name)
{
    struct config_node *ret;
    ret=malloc(sizeof *ret);
    ret->name=strdup(name);
    ret->type=NODETYPE_NIL;
    ret->next=0;
    ret->child=0;
    memset(&ret->value,0,sizeof ret->value);
    return ret;
}

static void straddch(char *str, int *idx, char ch)
{
    str[(*idx)++]=ch;
}

static int comment(FILE *f, int *line)
{
    int ch;    
    while((ch=fgetc(f))!=EOF) {
        if(ch=='\n') {
                        (*line)++;
            return 1; /* success */
        }
    }
    return EOF; /* end of file */
}

static int ws(FILE *f, int *line) /* parse whitespaces. just throw them away */
{
    int ch;
    strcpy(current_operation,"eating whitespace");
    while((ch=fgetc(f))!=EOF)
    {
        if(!isspace(ch)) {
                        /* # style comments */
            if(ch=='#') { /* comment, read to the end of line */
                if(comment(f,line)==EOF) return EOF;
                continue; /* look for some more whitespaces */
                        /* // style comments */
            } else if(ch=='/') {
                            ch=fgetc(f);
                            if(ch==EOF) return EOF;
                            if(ch=='/') { /* read to end of line */
                if(comment(f,line)==EOF) return EOF;
                continue; /* look for some more whitespaces */
                            }
                        }
            ungetc(ch,f); /* stuff it back in the buffer */
            return 1; /* success */
        } else if(ch=='\n') {
            (*line)++; /* increment line count */
        }
    }
    return EOF; /* EOF! probably done */
}

static int ident(FILE *f,char *buf,int buf_max)
{
    int bufi;
    int ch;

    bufi=0;
    while((ch=fgetc(f))!=EOF)
    {
        /* isalpha for first character, isalnum for rest */
        if((!bufi && isident_first(ch)) || (bufi && isident(ch))) {
            straddch(buf,&bufi,ch);
        } else {
            ungetc(ch,f); /* stuff it back in the buffer */
            break;
        }
    }
    straddch(buf,&bufi,0);
    /* EOF: done
     * found some data? : ok
     * no data : parse error
     */
    return ch==EOF ? EOF : bufi!=0; 
}

static int chm(FILE *f, int ch)
{
    int ch_tmp;
    snprintf(current_operation,sizeof current_operation,"looking for character '%c'",ch);
    ch_tmp=fgetc(f);
    if(ch_tmp==EOF) return EOF; /* EOF */
    if(ch_tmp==ch) {
        return 1;
    } else {
        ungetc(ch_tmp,f); /* stuff it back in the buffer */
        return 0;
    }
}

static int num(FILE *f, struct config_node *newchild)
{
    int total;
    int ret;
    int ch;
    int neg;
    strcpy(current_operation,"parsing a number");
    total=0;
    ret=0;
    neg=0;
    while((ch=fgetc(f))!=EOF)
    {
        if(ch=='-' && ret==0) {
            neg=1;
        } else if(isdigit(ch)) {
            ret=1; /* we read at least 1 thing, this will be successful */
            total=total*10+(ch-'0');
        } else {
            ungetc(ch,f); /* stuff it back in the buffer */
            break;
        }
    }
    if(ch==EOF) return EOF;
    if(ret) {
        if(neg) total= -total;

		newchild->type=NODETYPE_NUMBER;
		newchild->value.number=total;
        return 1;
    } else {
        return 0;
    }
}

/* Failure means the entire tree should be ignored. (we fill it in as we go,
 * and don't mark where we failed in the tree).
 */
static int expr(FILE *f,int *line, struct config_node **head, struct config_node **prev)
{
    int result;
    char buf[IDENT_MAX]; /* holds the identifier */
    struct config_node *newchild;
    int ch;

    OPTIONAL( ws(f,line) );
    MUST( ident(f,buf,sizeof buf) );
    newchild=createnode(buf);
    if(*prev) (*prev)->next=newchild; else *head=newchild;
    *prev=newchild;
    MUST( ws(f,line) );
    OPTIONAL( chm(f,'=') );
    MUST( ws(f,line) );

    ch=fgetc(f); /* peek some input */
    if(ch==EOF) return 0; /* cannot EOF here ! */
    switch(ch) {
        case '{': /* it's a sub-structure */
            {
                struct config_node *curr=0;
                strcpy(current_operation,"parsing a structure");
                while(1) {
                    result=expr(f,line,&newchild->child,&curr);
                    if(result<=0) return 0; /* can't EOF here! */
                    result=chm(f,'}');
                    if(result==1) break; /* found the other '}' */
                }
            }
            break;
        case '"': /* it's a string */
            {
                char str[STRING_MAX];
                int stri;
                int ch_tmp;
                strcpy(current_operation,"parsing a quoted string");
                stri=0;
                while((ch_tmp=fgetc(f))!=EOF) {
                    if(ch_tmp=='"') goto found_quote;
                    if(ch_tmp=='\n') (*line)++; /* increase line count */
                    straddch(str,&stri,ch_tmp);
                }
                return 0; /* EOF in the middle of a string */
                found_quote:
                straddch(str,&stri,0);
                DEBUG("STR: '%s'=\"%s\"\n",buf,str); /* TODO: put in tree */
				newchild->type=NODETYPE_STRING;
				newchild->value.str=strdup(str);
            }
            break;
        case '0': /* it's a number */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ungetc(ch,f); /* stuff the digit back in the buffer */
            MUST( num(f, newchild) );
            break;
    }
    OPTIONAL( chm(f,';') ); /* semi-colons are annoying */
    OPTIONAL( ws(f,line) );
    return 1;
}

static void dumptree(struct config_node *root,int depth)
{
    struct config_node *curr;

    for(curr=root;curr;curr=curr->next) {
        printf("%*s|\n",depth*4,"");
        printf("%*s+-- %s\n",depth*4,"",curr->name);
        if(curr->child) dumptree(curr->child,depth+1);
    }
}

/*** EXPORTED FUNCTIONS ***/
struct config_node *config_find(struct config_node *root, const char *name)
{
   for(;root&&strcmp(root->name,name);root=root->next) ;
   return root;
}

void config_free(struct config_node *root)
{
    struct config_node *curr;

    for(curr=root;curr;) {
        struct config_node *next;

        if(curr->child) config_free(curr->child);
        next=curr->next;
        free(curr->name);
        free(curr);
        curr=next;
    }
}


struct config_node *config_parser(const char *filename)
{
	FILE *f;
    int line;
    int result;
    struct config_node *root = 0;
    struct config_node *curr = 0;
    line=1;

        f=fopen(filename,"r");
        if(!f) {
            ERROR("Could not open '%s': %s\n",filename,strerror(errno));
            return 0;
        }
    while((result=expr(f,&line,&root,&curr))>0) ;

    if(result==EOF) { 
        reset_current_operation();
        DEBUG("Success!\n");
        return root; /* success */
    } else if(result==0) {
        ERROR("Line %d:Parse Error while %s\n",line,current_operation);
    } else if(result==1) {
        ERROR("Line %d:Trailing garbage\n",line);
    }
    /* TODO: Free the whole thing */
    return 0;
}

int config_get_str(struct config_node *item,char *buf,size_t buf_max)
{
    if(item && item->type==NODETYPE_STRING && strlen(item->value.str)<buf_max) {
        strcpy(buf,item->value.str);
        return 1;
    }
    return 0;
}

int config_get_int(struct config_node *item,int *i)
{
    if(item && item->type==NODETYPE_NUMBER) {
        *i=item->value.number;
        return 1;
    }
    return 0;
}

#if 0 /* EXAMPLE */
int main()
{
    struct config_node *root;
    
    root=parser(stdin);
    dumptree(root,0);
    return 0;
}

#endif /* EXAMPLE */
