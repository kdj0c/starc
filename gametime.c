/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <time.h>

static struct timespec off;

static inline struct timespec tssub(struct timespec a, struct timespec b) {
	struct timespec c;

	if (a.tv_nsec < b.tv_nsec) {
		c.tv_sec = a.tv_sec - b.tv_sec - 1;
		c.tv_nsec = 1000000000 + a.tv_nsec - b.tv_nsec;
	} else {
		c.tv_sec = a.tv_sec - b.tv_sec;
		c.tv_nsec = a.tv_nsec - b.tv_nsec;
	}
	return c;
}

void gtInit(void) {
	clock_gettime(CLOCK_MONOTONIC, &off);
//off.tv_nsec = 0;
//off.tv_sec = 0;
}

float gtGetTime(void) {
	struct timespec ts;
	float t;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts = tssub(ts, off);
	t = (float) ts.tv_sec * 1000. + ((float) ts.tv_nsec / 1000000.f);
	return t;
}

void gtSetOffset(float offset) {
	struct timespec o;
	long long int off64;
	int sign;

	if (offset >= 0)
		sign = 1;
	else {
		sign = -1;
		offset = -offset;
	}

	off64 = (long long int) (offset * 1000000.f);

	o.tv_sec = off64 / 1000000000;
	o.tv_nsec = off64 % 1000000000;

	if (sign > 0) {
		off.tv_sec += o.tv_sec;
		off.tv_nsec += o.tv_nsec;
		if (off.tv_nsec > 1000000000) {
			off.tv_nsec -= 1000000000;
			off.tv_sec++;
		}
	} else {
		off = tssub(off, o);
	}
}
