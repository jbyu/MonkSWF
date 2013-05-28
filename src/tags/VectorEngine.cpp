#include "VectorEngine.h"
#include "DefineShape.h"
#include "..\poly2tri\poly2tri.h"

namespace triangulation {

using namespace MonkSWF;

struct Collector;

typedef std::vector<p2t::Point>		PointDataArray;
typedef std::vector<p2t::Point*>	PointRefArray;
typedef std::vector<p2t::Triangle*> TriangleRefArray;
typedef std::vector<Collector*>		CollectorArray;

// Curve subdivision error tolerance.
static float s_tolerance = 4.0f;
static CollectorArray		s_collectors;
static POINTf				s_last_point;

const float SWF_EPSILON = 0.1f;

inline bool overlap(const p2t::Point* p, const p2t::Point* q) {
	if( (fabs( p->x - q->x ) < SWF_EPSILON) && 
		(fabs( p->y - q->y ) < SWF_EPSILON) )
		return true;
	return false;
}

//-----------------------------------------------------------------------------

struct Collector
{
	Collector()
		:_left_style(kINVALID)
		,_right_style(kINVALID)
		,_line_style(kINVALID)
		,_closed(false)
		,_clockwise(false)
	{
		_bound = kRectangleInvalidate;
	}

	Collector(int left_style, int right_style, int line_style)
		:_left_style(left_style)
		,_right_style(right_style)
		,_line_style(line_style)
		,_closed(false)
		,_clockwise(false)
	{
		_bound = kRectangleInvalidate;
	}

	int	_left_style;
	int _right_style;
	int _line_style;
	bool _closed;
	bool _clockwise;
	RECT _bound;

	CollectorArray	_holes;
	PointDataArray	_points;
	PointRefArray	_polyline;

	bool checkClosed(void) const {
		if (_closed || _right_style == kINVALID || _polyline.size() == 0) 
			return true;
		return false;
	}
#if 0
	void cleanEdges(void) {
		for (PointRefArray::iterator e = _polyline.begin(); e != _polyline.end(); ++e) 
			(*e)->edge_list.clear();
	}
	bool isClockWise(void) const {
		MK_ASSERT( 1 < _points.size() );
		float sum = 0;
		PointDataArray::const_iterator i = _points.begin();
		const p2t::Point* p0 = &(*i++);
		const p2t::Point* p1 = &(*i++);
		for ( ; i != _points.end(); ++i ) {
			const p2t::Point* p2 = &(*i);
			POINTf v1,v2;
			v1.x = p2->x - p1->x;
			v1.y = p2->y - p1->y;
			v2.x = p1->x - p0->x;
			v2.y = p1->y - p0->y;
			sum += v1.x * v2.y - v2.x * v1.y;
			p0 = p1;
			p1 = p2;
		}
		return 0 > sum;
	}
#endif
};

//-----------------------------------------------------------------------------

#if 0
inline float calcQuadraticBezier( float p0, float p1, float p2, float t ) {
	const float oneMinusT = 1.0f - t;
	float p = oneMinusT * oneMinusT * p0
		+ 2.0f * oneMinusT * t * p1
		+ t * t * p2;
	return p;	
}

inline POINTf calcQuadraticBezier( const POINTf& x0, const POINTf& x1, const POINTf& x2, float t ) {
	POINTf result;
	result.x = calcQuadraticBezier(x0.x, x1.x, x2.x, t);
	result.y = calcQuadraticBezier(x0.y, x1.y, x2.y, t);
	return result;	
}
#endif

void add_line_segment(const POINTf& pt)
// Add a line running from the previous anchor point to the
// given new anchor point.
{
	s_collectors.back()->_points.push_back( p2t::Point( pt.x, pt.y) ); // collect points
	s_collectors.back()->_bound.addPoint(pt); // update boundary
	s_last_point = pt;
}

static void	curve(const POINTf& p0, const POINTf& p1, const POINTf& p2)
// Recursive routine to generate bezier curve within tolerance.
{
#ifdef _DEBUG
	static int	recursion_count = 0;
	recursion_count++;
	if (recursion_count > 256)
		assert(0);	// probably a bug!
#endif//_DEBUG

	// Midpoint on line between two endpoints.
	POINTf mid = (p0 + p2) * 0.5f;
	// Midpoint on the curve.
	POINTf sample = (mid + p1) * 0.5f;

	mid -= sample;
	if (mid.magnitude() < s_tolerance) {
		add_line_segment(p2);
	} else {
		// Error is too large; subdivide.
		curve(p0,  (p0 + p1) * 0.5f, sample);
		curve(sample, (p1 + p2) * 0.5f, p2);
	}

#ifdef _DEBUG
	recursion_count--;
#endif//_DEBUG
}

void begin_path(Path& path)
// This call begins recording a sequence of segments, which
// all share the same fill & line styles.  Add segments to the
// shape using add_curve_segment() or add_line_segment(), and
// call end_path() when you're done with this sequence.
//
// Pass in -1 for styles that you want to disable.  Otherwise pass in
// the integral ID of the style for filling, to the left or right.
{
	Collector *set = new Collector(path._fill0, path._fill1, path._line);
	set->_clockwise = path.isClockWise();
	s_collectors.push_back(set);
	add_line_segment( path.getStart() );
}

void add_segment( const POINTf& anchor, const POINTf& ctrl )
// Add a curve segment to the shape.  The curve segment is a
// quadratic bezier, running from the previous anchor point to
// the given new anchor point, with the control point in between.
{
	if (anchor == ctrl) {
		// Early-out for degenerate straight segments.
		add_line_segment(anchor);
	} else {
		// Subdivide, and add line segments...
		curve(s_last_point, ctrl, anchor);
	}
}

void end_path(MonkSWF::ShapeWithStyle& shape)
// Mark the end of a set of edges that all use the same styles.
{
	Collector& it = *(s_collectors.back());
	//it._clockwise = it.isClockWise();

	// copy points to line vertices
	if (it._line_style > kINVALID && it._points.size() > 1) {
		MK_TRACE("add LINE: %d vertices\n", it._points.size());
		shape.addLine(it._line_style);
		for (PointDataArray::iterator pt = it._points.begin(); pt != it._points.end(); ++pt) {
			POINTf vtx = {pt->x, pt->y};
			shape.addLineVertex( vtx );
		}
	}

	// Convert left-fill paths into new right-fill paths,
	// so we only have to deal with right-fill below.
	if (kINVALID == it._right_style) {
		if (kINVALID == it._left_style) {
			return;
		}
		//it._hole = true;
		it._clockwise = !it._clockwise;
		it._right_style = it._left_style;
		it._left_style = kINVALID;
		// gather points for polyline in reverse order
		for (PointDataArray::reverse_iterator pt = it._points.rbegin(); pt != it._points.rend(); ++pt) {
			it._polyline.push_back( &(*pt) );
		}
	} else {
		// gather points for polyline in normal order
		for (PointDataArray::iterator pt = it._points.begin(); pt != it._points.end(); ++pt) {
			it._polyline.push_back( &(*pt) );
		}
		if (kINVALID == it._left_style) {
			return;
		}
		if (it._left_style == it._right_style) {
			// TODO: add steiner points
			return;
		}
		s_collectors.push_back( new Collector(kINVALID, it._left_style, it._line_style) );
		Collector &clone = *(s_collectors.back());
		//clone._hole = true;
		clone._clockwise = !it._clockwise;
		clone._bound = it._bound;
		clone._points = it._points; // duplicate points
		it._left_style = kINVALID;
		// gather points for polyline in reverse order
		for (PointDataArray::reverse_iterator pt = clone._points.rbegin(); pt != clone._points.rend(); ++pt) {
			clone._polyline.push_back( &(*pt) );
		}
	}
}//end_path()

//-----------------------------------------------------------------------------

void begin_shape(float curve_error_tolerance) {
	// ensure we're not already in a shape or path.
	// make sure our shape state is cleared out.
	MK_ASSERT(s_collectors.size() == 0);
	MK_ASSERT(curve_error_tolerance > 0);
	if (curve_error_tolerance > 0) {
		s_tolerance = curve_error_tolerance;
	}
}

void add_collector(MonkSWF::ShapeWithStyle& shape, MonkSWF::Path& path) {
	MonkSWF::Path::EdgeArray& _edges = path.getEdges();
	begin_path(path);
	for(MonkSWF::Path::EdgeArray::iterator it = _edges.begin(); it != _edges.end(); ++it) {
		add_segment(it->getAnchor(), it->getControl());
	}
	end_path(shape);
}

#if 0

// Return true if we did any work.
bool try_to_combine_path(Collector* pp)
{
	if ( pp->checkClosed() ) {
		return false;
	}

	if (overlap( pp->_polyline.back(), pp->_polyline.front() ) ) {
		pp->_polyline.pop_back();
		pp->_closed = true;
		return true;
	}

	// Look for another unclosed path of the same style,
	// which could join our begin or end point.
	const int style = pp->_right_style;
	for (CollectorArray::iterator it = s_collectors.begin(); it != s_collectors.end(); ++it) {
		Collector* po = (*it);
		if ( pp == po )
			continue;
		if ( po->checkClosed() ) 
			continue;
		if ( style != po->_right_style )
			continue;

		// Can we join?
		PointRefArray::iterator pStart = pp->_polyline.begin();
		PointRefArray::iterator oStart = po->_polyline.begin();
		if (overlap( *oStart, pp->_polyline.back() ) ) {
			// Yes, po can be appended to pp.
			pp->_polyline.insert( pp->_polyline.end(), ++oStart, po->_polyline.end());
			po->_right_style = kINVALID;
			return true;
		} else if (overlap( *pStart, po->_polyline.back() ) ) {
			// Yes, pp can be appended to po.
			po->_polyline.insert( po->_polyline.end(), ++pStart, pp->_polyline.end());
			pp->_right_style = kINVALID;
			return true;
		}
	}

	return false;
}
	
//-----------------------------------------------------------------------------

void end_shape(MonkSWF::ShapeWithStyle& shape) {
	// Join path_parts together into closed paths.
	while (1) {
		bool did_work = false;
		for (CollectorArray::iterator it = s_collectors.begin(); it != s_collectors.end(); ++it) {
			if (try_to_combine_path( (*it) )) {
				did_work = true;
				break;
			}
		}
		if (did_work == false) {
			break;
		}
	}
		
	// Triangulate and emit.
	for (CollectorArray::iterator it = s_collectors.begin(); it != s_collectors.end(); ++it) {
		Collector* pp = (*it);
		if (NULL == pp ||
			false == pp->_closed ||
			true == pp->_hole ||
			kINVALID == pp->_right_style ||
			0 == pp->_polyline.size() )
			continue;
		
		// add a new mesh
		shape.addMesh(pp->_right_style);

		// clean edges info of shared points
		for (PointRefArray::iterator e = pp->_polyline.begin(); e != pp->_polyline.end(); ++e) {
			(*e)->edge_list.clear();
		}
		// allocate a triangulator
		p2t::CDT* cdt = new p2t::CDT(pp->_polyline);

		// add holes
		const int style = pp->_right_style;
		for (CollectorArray::iterator oit = s_collectors.begin(); oit != s_collectors.end(); ++oit) {
			Collector* po = (*oit);
			if (NULL == po ||
				pp == po ||
				false == po->_closed ||
				true != po->_hole ||
				style != po->_right_style )
				continue;

			// clean edges info of shared points
			for (PointRefArray::iterator e = po->_polyline.begin(); e != po->_polyline.end(); ++e) {
				(*e)->edge_list.clear();
			}
			cdt->AddHole(po->_polyline);
		}
		// process
		cdt->Triangulate();

		// TODO: convert to triangle-strip
		TriangleRefArray triangles = cdt->GetTriangles();
		MK_TRACE("add MESH [fill:%d]: %d triangles\n", pp->_right_style, triangles.size());

		for (TriangleRefArray::iterator it = triangles.begin(); it != triangles.end(); ++it) {
			p2t::Triangle* tri = *it;
			for (int i = 0; i < 3; ++i) {
				POINTf pt = {tri->GetPoint(i)->x, tri->GetPoint(i)->y};
				shape.addMeshVertex(pt);
			}
			MK_TRACE("triangle:[%.2f,%.2f],[%.2f,%.2f],[%.2f,%.2f]\n",
				tri->GetPoint(0)->x, tri->GetPoint(0)->y,
				tri->GetPoint(1)->x, tri->GetPoint(1)->y,
				tri->GetPoint(2)->x, tri->GetPoint(2)->y);
		}
		// free the triangulator
		delete cdt;
	}
	// clean memory
	for (CollectorArray::iterator it = s_collectors.begin(); it != s_collectors.end(); ++it) {
		delete (*it);
	}
	s_collectors.clear();
}
#else
//-----------------------------------------------------------------------------

CollectorArray::iterator find_connecting( CollectorArray::iterator& target, CollectorArray& input )
{
	CollectorArray::iterator ret = input.end();
	Collector* pp = *target;
	if ( pp->checkClosed() ) {
		return ret;
	}

	if (overlap( pp->_polyline.back(), pp->_polyline.front() ) ) {
		// poly2tri only accept polyline with non repeating points
		pp->_polyline.pop_back();
		pp->_closed = true;
		return ret;
	}

	// Look for another unclosed path of the same style,
	// which could join our begin or end point.
	for (CollectorArray::iterator it = input.begin(); it != input.end(); ++it) {
		Collector* po = (*it);
		if ( pp == po )
			continue;
		if ( po->checkClosed() ) 
			continue;

		// Can we join?
		PointRefArray::iterator pStart = pp->_polyline.begin();
		PointRefArray::iterator oStart = po->_polyline.begin();
		if (overlap( *oStart, pp->_polyline.back() ) ) {
			// Yes, po can be appended to pp.
			pp->_polyline.insert( pp->_polyline.end(), ++oStart, po->_polyline.end());
			po->_right_style = kINVALID;
			MergeRectangle(pp->_bound, po->_bound);
			ret = it; // remove this iterator and restart 
			return ret;
		} else if (overlap( *pStart, po->_polyline.back() ) ) {
			// Yes, pp can be appended to po.
			po->_polyline.insert( po->_polyline.end(), ++pStart, pp->_polyline.end());
			pp->_right_style = kINVALID;
			MergeRectangle(po->_bound, pp->_bound);
			ret = target; // remove this iterator and restart 
			return ret;
		}
	}

	return ret;
}

void get_contours_and_holes( CollectorArray& input ) {
	// join collectors together into closed paths.
	while (1) {
		bool found_connection = false;
		for (CollectorArray::iterator it = input.begin(); it != input.end(); ++it) {
			CollectorArray::iterator result = find_connecting( it, input );
			if (input.end() != result) {
				input.erase( result );
				found_connection = true;
				break;
			}
		}
		if (false == found_connection)
			break;
	}

	// find holes
	while (1) {
		bool found = false;
		for (CollectorArray::iterator i = input.begin(); i != input.end(); ++i) {
			for (CollectorArray::iterator j = input.begin(); j != input.end(); ++j) {
				Collector* cA = *i;
				Collector* cB = *j;
				if ( cA == cB )	// don't check against self
					continue;
				// inner has to be counter direction to outer
				if ( cA->_clockwise == cB->_clockwise ) 
					continue;
				
				if ( cB->_bound.isInside(cA->_bound) ) {
					cA->_holes.push_back(cB);
					input.erase(j);
					found = true;
					break;
				}
			}
			if ( found )
				break;
		}
		if (false == found)
			break;
	}
}

bool create_mesh( MonkSWF::ShapeWithStyle& shape, Collector* pp ) {
	if (NULL == pp ||
		false == pp->_closed ||
		kINVALID == pp->_right_style ||
		0 == pp->_polyline.size() )
		return false;
		
	// add a new mesh
	shape.addMesh(pp->_right_style);
	// clean edges info of shared points
	//pp->cleanEdges();
	// allocate a triangulator
	p2t::CDT* cdt = new p2t::CDT(pp->_polyline);
	// add holes
	const int style = pp->_right_style;
	for (CollectorArray::iterator oit = pp->_holes.begin(); oit != pp->_holes.end(); ++oit) {
		Collector* po = (*oit);
		if (NULL == po || pp == po)
			continue;
		// clean edges info of shared points
		//po->cleanEdges();
		cdt->AddHole(po->_polyline);
	}
	// process
	cdt->Triangulate();

	// TODO: convert to triangle-strip
	TriangleRefArray triangles = cdt->GetTriangles();
	MK_TRACE("add MESH [fill:%d]: %d triangles\n", pp->_right_style, triangles.size());
	for (TriangleRefArray::iterator it = triangles.begin(); it != triangles.end(); ++it) {
		p2t::Triangle* tri = *it;
		for (int i = 0; i < 3; ++i) {
			POINTf pt = {tri->GetPoint(i)->x, tri->GetPoint(i)->y};
			shape.addMeshVertex(pt);
		}
		MK_TRACE("triangle:[%.2f,%.2f],[%.2f,%.2f],[%.2f,%.2f]\n",
			tri->GetPoint(0)->x, tri->GetPoint(0)->y,
			tri->GetPoint(1)->x, tri->GetPoint(1)->y,
			tri->GetPoint(2)->x, tri->GetPoint(2)->y);
	}
	// free the triangulator
	delete cdt;
	return true;
}

void end_shape(MonkSWF::ShapeWithStyle& shape) {
	int size = shape.getFillStyles().size();
	for (int i = 0; i < size; ++i) {
		CollectorArray contours;
		contours.reserve(4);

		// collect same style contours
		for (CollectorArray::iterator it = s_collectors.begin(), end = s_collectors.end(); it != end; ++it) {
			Collector* current = *it;
			if ( current->_right_style == i ) {
				contours.push_back( current );
			}
		}
		if (0 == contours.size())
			continue;

		// connect contours and find holes
		get_contours_and_holes(contours);
		
		// Triangulate and emit.
		for (CollectorArray::iterator it = contours.begin(); it != contours.end(); ++it) {
			create_mesh( shape, *it );
		}
	}
	// clean memory
	for (CollectorArray::iterator it = s_collectors.begin(); it != s_collectors.end(); ++it) {
		delete (*it);
	}
	s_collectors.clear();
}
#endif
//-----------------------------------------------------------------------------

}// end namespace triangulation
