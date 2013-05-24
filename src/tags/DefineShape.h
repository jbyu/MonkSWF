/*
 *  DefineShape.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/21/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __DefineShape_h__
#define __DefineShape_h__

#include "mkTag.h"
#include "VectorEngine.h"

namespace MonkSWF {

class DefineShapeTag;

//-----------------------------------------------------------------------------

class Gradient {
public:
	bool read( Reader* reader, bool support_32bit_color );
    //void configPaint( VGPaint paint, uint8_t type, MATRIX& m, bool support_32bit_color);
		
private:
	uint8_t		_spread_mode;			// 0 = Pad mode 1 = Reflect mode 2 = Repeat mode
	uint8_t		_interpolation_mode;	// 0 = Normal RGB mode interpolation 1 = Linear RGB mode interpolation
	uint8_t		_num_gradients;
		
	struct Record {
		uint8_t	_ratio;
		RGBA	_color;
	};
		
	typedef std::vector< Gradient::Record > GradientRecordArray;
	typedef GradientRecordArray::iterator GradientRecordArrayIter;
		
	GradientRecordArray		_gradient_records;
};

//-----------------------------------------------------------------------------

class Edge {
public:
	Edge( const POINTf& anchor ) 
		:_anchor(anchor)
		,_control(anchor)
	{}

	Edge( const POINTf& anchor, const POINTf& ctrl ) 
		:_anchor(anchor)
		,_control(ctrl)
	{}

	POINTf& getAnchor() {
		return _anchor;
	}

	POINTf& getControl() {
		return _control;
	}

	void print() const {
		if (_anchor == _control) {
			MK_TRACE("[line]  anchor:%.2f, %.2f\n", _anchor.x, _anchor.y);
		} else {
			MK_TRACE("[curve] anchor:%.2f, %.2f;\tctrl:%.2f, %.2f\n", _anchor.x, _anchor.y, _control.x, _control.y);
		}
	}

protected:		
	POINTf	_anchor;
	POINTf	_control;
};
	
//-----------------------------------------------------------------------------

class Path {
public:
	typedef std::vector<Edge>	EdgeArray;

	Path()
		:_fill0( kINVALID )
		,_fill1( kINVALID )
		,_line( kINVALID )
		,_new_shape(false)
	{
		_start.x = 0.f;
		_start.y = 0.f;
	}

	void addEdge( const Edge& e ) {
		MK_TRACE("add ");
		e.print();
		_edges.push_back( e );
	}
	
	bool isEmpty() const {
		return _edges.size() == 0;
	}

	POINTf& getStart() { return _start; }

	EdgeArray& getEdges() { return _edges; }

	// style indices
	int _fill0;
	int _fill1;
	int _line;
	bool _new_shape;

private:
	POINTf		_start;
	EdgeArray	_edges;
};

//-----------------------------------------------------------------------------

class ShapeWithStyle {
public:
	bool read( Reader& reader, SWF&, DefineShapeTag* define_shape_tag );
	
	void draw();

	void addMesh(size_t fill_idx) {
		MK_ASSERT(0 <= fill_idx && _fill_styles.size() > fill_idx);
		Mesh mesh = { &_fill_styles[fill_idx], {false,0,{0,0,0,0}}};
		_meshes.push_back(mesh);
	}

	void addMeshVertex( const POINTf& pt) {
		_meshes.back()._vertices.push_back(pt);
	}

	void addLine(size_t line_idx) {
		MK_ASSERT(0 <= line_idx && _line_styles.size() > line_idx);
		Line l = { &_line_styles[line_idx] };
		_lines.push_back(l);
	}

	void addLineVertex( const POINTf& pt) {
		_lines.back()._vertices.push_back(pt);
	}

	bool isInside(float x, float y) const;

private:
	struct Mesh {
		FillStyle	*_style;
		Asset		_asset;
		RECT		_bound;
		VertexArray	_vertices;

		bool isInsideMesh(const POINTf& pt) const;
	};
	struct Line {
		LineStyle	*_style;
		VertexArray	_vertices;
	};
	typedef std::vector<Mesh>		MeshArray;
	typedef std::vector<Line>		LineArray;
	typedef std::vector<FillStyle>	FillStyleArray;
	typedef std::vector<LineStyle>	LineStyleArray;
	typedef std::vector<Path>		PathArray;

	FillStyleArray	_fill_styles;
	LineStyleArray	_line_styles;		
	PathArray		_paths;
	MeshArray		_meshes;
	LineArray		_lines;

	bool readStyles(Reader* reader, bool lineStyle2, bool support_32bit_color);

	bool readShapeRecords(Reader* reader, bool lineStyle2, bool support_32bit_color);

	bool triangluate();
};
	
//-----------------------------------------------------------------------------

class DefineShapeTag : public ITag, public ICharacter {
public:
	DefineShapeTag( TagHeader& h )
		:ITag( h )
        ,_shape_id(0)
	{
    }
		
	virtual ~DefineShapeTag()
    {
	}
	
	virtual bool read( Reader& reader, SWF&, MovieFrames& data );
	virtual void print();
		
	// override ICharacter function
    virtual const RECT& getRectangle(void) const { return _bound; }
	virtual void draw(void);
	virtual void update(void) {}
    virtual void play( bool enable ) {}
	virtual void gotoFrame( uint32_t frame ) {}
	virtual ICharacter* getTopMost(float localX, float localY, bool polygonTest);
		
	static ITag* create( TagHeader& header );
	
protected:
	uint16_t		_shape_id;
	RECT			_bound;
	ShapeWithStyle	_shape_with_style;
};
	
//-----------------------------------------------------------------------------

} //namespace

#endif // __DefineShape_h__
