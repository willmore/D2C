/*
 * fdstream.h: generic streams impl for file descriptors
 *
 * Copyright (C) 2009-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */


#ifndef __VIR_FDSTREAM_H_
# define __VIR_FDSTREAM_H_

# include "internal.h"
# include <stdbool.h>

int virFDStreamOpen(virStreamPtr st,
                    int fd);

int virFDStreamConnectUNIX(virStreamPtr st,
                           const char *path,
                           bool abstract);

int virFDStreamOpenFile(virStreamPtr st,
                        const char *path,
                        int flags);
int virFDStreamCreateFile(virStreamPtr st,
                          const char *path,
                          int flags,
                          mode_t mode);

#endif /* __VIR_FDSTREAM_H_ */
