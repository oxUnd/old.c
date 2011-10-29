#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "global.h"

int open_clientfd(const char *hostname, unsigned int port, int time);

#endif
