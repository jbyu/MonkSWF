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

const int kINVALID = -1;

class ShapeWithStyle;
class Path;

}

//-----------------------------------------------------------------------------

namespace triangulation {
	void begin_shape(float curve_error_tolerance);
	void add_collector(MonkSWF::ShapeWithStyle& shape, MonkSWF::Path&);
	void end_shape(MonkSWF::ShapeWithStyle&);
}

//-----------------------------------------------------------------------------	

#endif//__VECTOR_ENGINE__