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

namespace MonkSWF {

void Log(const char* szmsg, ...);

#ifdef SWF_DEBUG
#define MK_ASSERT	assert
#define MK_TRACE	Log
#else
    
#ifdef WIN32
#define MK_ASSERT	(void)
#define MK_TRACE	(void)
#else
#define MK_ASSERT(...)
#define MK_TRACE(...)
#endif    
    
#endif

}

#endif // __mkCommon_h__
