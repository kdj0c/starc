/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "save.h"
#include "event.h"

static int savefd = -1;

void saExit(void) {
    close(savefd);
}

void saInit(char *file) {
    savefd = open(file, O_WRONLY | O_CREAT | O_NONBLOCK | O_TRUNC, 0660 );
    if (savefd < 0) {
    	printf("cannot open %s", file);
    	return;
    }
    atexit(saExit);
}

void saSaveEvent(ev_t *ev) {
    int n;
    if (savefd < 0)
        return;
    n = write(savefd, ev, sizeof(*ev));
    n = write(savefd, ev->data, ev->size);

    if (n < 0)
    	printf("error writing replay\n");
}

void saReplay(char *file) {
    int fd;
    ev_t ev;
    char buf[256];
    int n;

    fd = open(file, O_RDONLY);

    do {
        n = read(fd, &ev, sizeof(ev));
        n = read(fd, buf, ev.size);
        evPostEventLocal(ev.time, buf, ev.size, ev.type);
    } while (n > 0);
}
