/*
 * Copyright © 2015 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
 * SPDX-License-Identifier: MIT
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD6_GMEM_H_
#define FD6_GMEM_H_

#include "pipe/p_context.h"

template <chip CHIP>
void fd6_gmem_init(struct pipe_context *pctx);

#endif /* FD6_GMEM_H_ */
