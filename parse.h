/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

typedef enum {
    PS_OBJECT,
    PS_ARRAY,
    PS_STRING,
    PS_INTEGER,
    PS_FLOAT
} ps_type;

typedef struct ps_node {
    struct ps_node* parent;
    struct ps_node* child;
    struct ps_node* next;
    struct ps_node* lchild;
    ps_type type;
    const char* key;
    const char* value_s;
    int value_i;
    float value_f;
    int len;

} ps_node;

struct ps_node *psParseFile(const char *filename);
char *psReadfile(const char *filename);
void psFreeNodes(struct ps_node *nd);
struct ps_node *psGetObject(const char *name, struct ps_node* nd);
int psGetInt(const char *name, struct ps_node* nd);
const char *psGetStr(const char *name, struct ps_node* nd);
float psGetFloat(const char *name, struct ps_node* nd);
#endif /* CONFIG_H_ */
