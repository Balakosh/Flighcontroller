/*
 * version.h
 *
 *  Created on: Mar 27, 2022
 *      Author: Akeman
 */

#ifndef VERSION_VERSION_H_
#define VERSION_VERSION_H_

inline unsigned int getVersionMajor(void);
inline unsigned int getVersionMinor(void);
inline unsigned int getVersionPatch(void);
char* getGitInfo(void);

#endif /* VERSION_VERSION_H_ */
