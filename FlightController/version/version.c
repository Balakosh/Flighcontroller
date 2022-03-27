/*
 * version.c
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#include <stdio.h>

static const unsigned int versionMajor = 1;
static const unsigned int versionMinor = 0;
static const unsigned int versionPatch = 0;

char gitVersionString[256];

inline unsigned int getVersionMajor(void)
{
    return versionMajor;
}

inline unsigned int getVersionMinor(void)
{
    return versionMinor;
}

inline unsigned int getVersionPatch(void)
{
    return versionPatch;
}

char* getGitInfo(void)
{
    snprintf(gitVersionString, sizeof(gitVersionString), "gitinfo");

    return gitVersionString;
}
