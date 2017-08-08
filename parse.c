/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "parse.h"

static char *cfgbuf = NULL;

enum {
	PS_COMMENT,
	PS_SEEK_KEY,
	PS_KEY,
	PS_SEEK_VALUE,
	PS_VALUE
};

struct ps_node *createNode(ps_type type, const char *key, struct ps_node *parent) {
	struct ps_node *nd;

	nd = malloc(sizeof(*nd));
	nd = memset(nd, 0, sizeof(*nd));
	nd->type = type;
	nd->key = key;
	nd->parent = parent;
	if (parent) {
		if (!parent->lchild) {
			parent->child = parent->lchild = nd;
		} else {
			parent->lchild->next = nd;
			parent->lchild = nd;
		}
		parent->len++;
	}
	return nd;
};

void printNodes(struct ps_node *nd, int level) {
	struct ps_node *cur;

	for (cur = nd; cur; cur = cur->next) {
		if (level)
			printf("%*s", level, "");
		printf("%d Node %s ", level, cur->key);
		switch (cur->type) {
		case PS_OBJECT:
			printf("Object\n");
			break;
		case PS_ARRAY:
			printf("Array\n");
			break;
		case PS_STRING:
			printf("= [str] %s\n", cur->value_s);
			break;
		case PS_INTEGER:
			printf("= [int] %d\n", cur->value_i);
			break;
		case PS_FLOAT:
			printf("= [flt] %f\n", cur->value_f);
			break;
		}
		printNodes(cur->child, level + 1);
	}
}

void psFreeNodes(struct ps_node *nd) {
	struct ps_node *cur;
	struct ps_node *next;
	cur = nd;
	while (cur) {
		psFreeNodes(cur->child);
		next = cur->next;
		free(cur);
		cur = next;
	}
	free(cfgbuf);
	cfgbuf = NULL;
}

char *psReadfile(const char *filename) {
	int fd;
	struct stat st;
	int count;
	int off;

	if (stat(filename, &st) < 0) {
		fprintf(stderr, "Failed to open config file : %s\n", filename);
		return NULL;
	}
	if ((fd = open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "Failed to open config file : %s\n", filename);
		return NULL;
	}

	cfgbuf = malloc(st.st_size + 1);
	if (!cfgbuf)
		return NULL;

	off = 0;
	do {
		count = read(fd, cfgbuf + off, st.st_size - off);
		if (count <= 0) {
			fprintf(stderr, "Failed to read config file : %d/%ld\n", count, st.st_size);
			return NULL;
		}
		off += count;
	} while (off < st.st_size && count != 0);
	close(fd);
	cfgbuf[st.st_size] = 0;
	return cfgbuf;
}

struct ps_node *psParseFile(const char *filename) {
	struct ps_node *root;
	struct ps_node *parent;
	struct ps_node *cur;
	char *data;
	char *p;
	char *startkey;
	char *startvalue;
	char c;
	int state;
	ps_type t;

	data = psReadfile(filename);
	if (!data)
		return NULL;

	root = createNode(PS_OBJECT, "root", NULL);
	cur = root;
	parent = root;

	p = data;
	state = PS_SEEK_KEY;

	for (p = data; *p; p++) {
		c = *p;
		switch (state) {
		case PS_COMMENT:
			if (c == '\n')
				state = PS_SEEK_KEY;
			break;
		case PS_SEEK_KEY:
			if (c == '#')
				state = PS_COMMENT;
			else if (c == '}' || c == ')') {
				cur = cur->parent;
				parent = cur->parent;
			} else if (c == '{') {
				cur = createNode(PS_OBJECT, startkey, parent);
				parent = cur;
				state = PS_SEEK_KEY;
			} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				startkey = p;
				state = PS_KEY;
			}
			break;
		case PS_KEY:
			if (c == ' ' || c == '\n' || c == '\t' || c == '=' || c == ':') {
				*p = 0;
				state = PS_SEEK_VALUE;
			}
			break;
		case PS_SEEK_VALUE:
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '"' || c == '-') {
				startvalue = p;
				state = PS_VALUE;
				t = PS_INTEGER;
			} else if (c == '{') {
				*p = 0;
				cur = createNode(PS_OBJECT, startkey, parent);
				parent = cur;
				state = PS_SEEK_KEY;
			} else if (c == '(') {
				*p = 0;
				cur = createNode(PS_ARRAY, startkey, parent);
				parent = cur;
				state = PS_SEEK_KEY;
			}
			break;
		case PS_VALUE:
			if (c == ' ' || c == '\n' || c == '\t' || c == ';' || c == '"') {
				*p = 0;
				if (*startvalue == '"') {
					cur = createNode(PS_STRING, startkey, parent);
					cur->value_s = startvalue + 1;
				} else if (t == PS_FLOAT) {
					cur = createNode(PS_FLOAT, startkey, parent);
					cur->value_f = strtof(startvalue, NULL);
				} else {
					cur = createNode(PS_INTEGER, startkey, parent);
					cur->value_i = (int) strtol(startvalue, NULL, 0);
				}
				state = PS_SEEK_KEY;
			} else if (c == '.')
				t = PS_FLOAT;
		}
	}
	// printNodes(root, 0);
	return root;
}

struct ps_node *psGetObject(const char *name, struct ps_node *nd) {
	struct ps_node *cur;
	if (!nd)
		return NULL;
	for (cur = nd->child; cur; cur = cur->next) {
		if (!strcmp(name, cur->key))
			return cur;
	}
	return NULL;
	fprintf(stderr, "Error, Node %s not found\n", name);
}

int psGetInt(const char *name, struct ps_node *nd) {
	struct ps_node *cur;
	if (!nd)
		return 0;
	for (cur = nd->child; cur; cur = cur->next) {
		if (!strcmp(name, cur->key)) {
			if (cur->type == PS_INTEGER)
				return cur->value_i;
			else
				fprintf(stderr, "Error, Node %s is not integer\n", name);
		}
	}
	fprintf(stderr, "Error, Node %s not found\n", name);
	return 0;
}

const char *psGetStr(const char *name, struct ps_node *nd) {
	struct ps_node *cur;
	if (!nd)
		return 0;
	for (cur = nd->child; cur; cur = cur->next) {
		if (!strcmp(name, cur->key)) {
			if (cur->type == PS_STRING)
				return cur->value_s;
			else
				fprintf(stderr, "Error, Node %s is not a string\n", name);
		}
	}
	fprintf(stderr, "Error, Node %s not found\n", name);
	return NULL;
}

float psGetFloat(const char *name, struct ps_node *nd) {
	struct ps_node *cur;
	if (!nd)
		return 0;
	for (cur = nd->child; cur; cur = cur->next) {
		if (!strcmp(name, cur->key)) {
			if (cur->type == PS_FLOAT)
				return cur->value_f;
			else
				fprintf(stderr, "Error, Node %s is not a float\n", name);
		}
	}
	fprintf(stderr, "Error, Node %s not found\n", name);
	return 0;
}
