#ifndef __VECTOR_ENGINE__
#define __VECTOR_ENGINE__

#include <math.h>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator> 
#include "mkCommon.h"
#include "mkTypes.h"

//-----------------------------------------------------------------------------

namespace MonkSWF {

class ShapeWithStyle;
class Path;

}

//-----------------------------------------------------------------------------

namespace triangulation {
	void create_memory_pool(int size);
	void destroy_memory_pool(void);
	void begin_shape(void);
	void add_collector(MonkSWF::ShapeWithStyle& shape, MonkSWF::Path&);
	void end_shape(MonkSWF::ShapeWithStyle&);
}

//-----------------------------------------------------------------------------	

#endif//__VECTOR_ENGINE__