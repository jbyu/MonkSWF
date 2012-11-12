/*
 *  mkCommon.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/18/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __mkCommon_h__
#define __mkCommon_h__

#include <cassert>
#include <cstddef>
#include <stdint.h>
#include <stdio.h>
#include <cmath>

#ifdef _DEBUG
#define MK_ASSERT	assert
//#define MK_TRACE	printf
#else
#define MK_ASSERT	(void)
//#define MK_TRACE	(void)
#endif

#define MK_TRACE	printf

#endif // __mkCommon_h__
