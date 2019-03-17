#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

/* 
 * Takes in a line and tokenizes it by the given delimiter. 
 */
char *next_token(char **str_ptr, const char *delim);

#endif
