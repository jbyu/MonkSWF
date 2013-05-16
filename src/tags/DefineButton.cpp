#include "DefineButton.h"
#include "PlaceObject.h"
#include "mkSWF.h"

namespace MonkSWF {

//-----------------------------------------------------------------------------
	
// Return true if we read a record; false if this is a null record.
bool ButtonRecord::read( Reader& reader, SWF& swf, int tag_type ) {
	uint8_t	flags = reader.get<uint8_t>();
	if (0 == flags)
		return false;
	/*
	_has_blend_mode	= flags & 0x20 ? true : false;
	_has_filter_list= flags & 0x10 ? true : false;
	_hit_test		= flags & 0x08 ? true : false;
	_down			= flags & 0x04 ? true : false;
	_over			= flags & 0x02 ? true : false;
	_up				= flags & 0x01 ? true : false;
	*/
	_state = flags;
	_character_id = reader.get<uint16_t>();
	_depth = reader.get<uint16_t>();
	reader.getMatrix(_matrix);

	if (TAG_DEFINE_BUTTON2 == tag_type) {
		reader.align();
		reader.getCXForm( _cxform );
		if ( flags & 0x10 ) {
			reader.getFilterList();
		}
		if ( flags & 0x20 ) {
			_blend_mode = reader.get<uint8_t>();
		}
	}

	return true;
}

void ButtonRecord::print() {
#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
	(byte & 0x80 ? 1 : 0), \
	(byte & 0x40 ? 1 : 0), \
	(byte & 0x20 ? 1 : 0), \
	(byte & 0x10 ? 1 : 0), \
	(byte & 0x08 ? 1 : 0), \
	(byte & 0x04 ? 1 : 0), \
	(byte & 0x02 ? 1 : 0), \
	(byte & 0x01 ? 1 : 0) 
	MK_TRACE("state:0x" BYTETOBINARYPATTERN " character=%d, depth=%d", BYTETOBINARY(_state), _character_id, _depth);
	MK_TRACE("\tmtx:%3.2f,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f\n",
		_matrix.m_[0][0], _matrix.m_[0][1], _matrix.m_[0][2],
		_matrix.m_[1][0], _matrix.m_[1][1], _matrix.m_[1][2]);
}

//-----------------------------------------------------------------------------

bool ButtonAction::read( Reader& reader, SWF& swf, MovieFrames& frames, int tag_type )
{
	// Read condition flags.
	if (TAG_DEFINE_BUTTON == tag_type) {
		_conditions = OVER_DOWN_TO_OVER_UP;
	} else {
		assert(tag_type == TAG_DEFINE_BUTTON2);
		_conditions = reader.get<uint16_t>();
	}

	// Read actions.
	return _actions.read( reader, swf, frames);
}

//-----------------------------------------------------------------------------

DefineButton2Tag::~DefineButton2Tag()
{
	for (size_t i = 0; i < _buttonActions.size(); ++i) {
		delete _buttonActions[i];
	}
}

bool DefineButton2Tag::read( Reader& reader, SWF& swf, MovieFrames& frames) 
{
	_buttonId = reader.get<uint16_t>();
	_asMenu = reader.get<uint8_t>() != 0;

	int pos = reader.getCurrentPos();
	uint16_t _actionOffset = reader.get<uint16_t>();

	RECT empty = {FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX};
	_bound = empty;

	// Read button records.
	while (1) {
		ButtonRecord rec;
		if (rec.read(reader, swf, TAG_DEFINE_BUTTON2) == false) {
			// Null record; marks the end of button records.
			break;
		}
		RECT rect = swf.calculateRectangle(rec._character_id, &rec._matrix);
		MergeRectangle(_bound, rect);

		_buttonRecords.push_back(rec);
	}

    swf.addCharacter( this, _buttonId );
	if (0 == _actionOffset)
		return true;

	int skip = _actionOffset + pos - reader.getCurrentPos();
	reader.skip(skip);
	
	while (1) {
		int curr = reader.getCurrentPos();
		int	actionSize = reader.get<uint16_t>();

		ButtonAction *action = new ButtonAction;
		action->read( reader, swf, frames, TAG_DEFINE_BUTTON2);
		_buttonActions.push_back(action);

		if (actionSize == 0)
			break;
	}
	return true; // keep tag
}

void DefineButton2Tag::print() {
    _header.print();
	MK_TRACE("id=%d\n", _buttonId);
	for (size_t i = 0; i < _buttonRecords.size(); ++i) {
		_buttonRecords[i].print();
	}
	for (size_t i = 0; i < _buttonActions.size(); ++i) {
		_buttonActions[i]->print();
	}
}

//-----------------------------------------------------------------------------

Button::Button( MovieClip& parent,  DefineButton2Tag& data )
	:MovieClip( parent.getSWF(), _frames )
	,_parent(parent)
	,_definition(data)
	,_mouseState(MOUSE_UP)
{
	MK_TRACE("create Button\n");
	// allocate PlaceObjects for button states
	ButtonRecordArray::const_iterator it = getDefinition()._buttonRecords.begin();
	while( getDefinition()._buttonRecords.end() != it) {
		PlaceObjectTag *object = new PlaceObjectTag( *it );
		object->setup(*this);
		ButtonState state = { it->_state, object };
		if (it->_state & kButtonStateHitTest) {
			_buttonHitTests.push_back( state );
		} else {
			_buttonStates.push_back( state );
		}
		++it;
	}
	clearDisplayList();
	setupFrame();
}

Button::~Button()
{
	MK_TRACE("delete Button\n");
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		delete it->_object;
		++it;
	}
	it = _buttonHitTests.begin();
	while( _buttonHitTests.end() != it) {
		delete it->_object;
		++it;
	}
}

#if 1

void Button::update(void)
{
	int flag = 1 << _mouseState;
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		if (it->_state & flag) {
			it->_object->update();
		}
		++it;
	}
}

void Button::play(bool enable) 
{
	int flag = 1 << _mouseState;
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		if (it->_state & flag) {
			it->_object->play(enable);
		}
		++it;
	}
}

void Button::gotoFrame(uint32_t frame)
{
	int flag = 1 << _mouseState;
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		if (it->_state & flag) {
			it->_object->gotoFrame( frame );
		}
		++it;
	}
}

void Button::draw() {
	int flag = 1 << _mouseState;
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		if (it->_state & flag) {
			it->_object->draw();
		}
		++it;
	}
}

ICharacter* Button::getTopMost(float x, float y) {
	StateArray::iterator it = _buttonHitTests.begin();
	while( it != _buttonHitTests.end() ) {
		PlaceObjectTag* pObj = it->_object;
		MK_ASSERT(pObj);
		ICharacter* pCharacter = pObj->getCharacter();
		if (pCharacter) {
			MATRIX m;
			POINTf local, world = {x,y};
			m.setInverse(pObj->getTransform());
			m.transform(local, world);
			if (pCharacter->getTopMost(local.x, local.y))
				return this;
		}
        ++it;
	}
	return NULL;
}

#endif 

void Button::setupFrame(void) {
	int flag = 1 << _mouseState;
	StateArray::const_iterator it = _buttonStates.begin();
	while( _buttonStates.end() != it) {
		if (it->_state & flag) {
			it->_object->setup(*this);
		}
		++it;
	}
}

void Button::onEvent(Event::Code code) {
	int flag = 0;
	switch(code) {
	case Event::ROLL_OUT:
		_mouseState = MOUSE_UP;
		flag |= (ButtonAction::OVER_UP_TO_IDLE);
		break;
	case Event::ROLL_OVER:
		_mouseState = MOUSE_OVER;
		flag |= (ButtonAction::IDLE_TO_OVER_UP);
		break;
	case Event::PRESS:
		_mouseState = MOUSE_DOWN;
		flag |= (ButtonAction::OVER_UP_TO_OVER_DOWN);
		break;
	case Event::RELEASE:
		_mouseState = MOUSE_UP;
		flag |= (ButtonAction::OVER_DOWN_TO_OVER_UP);
		break;
	case Event::RELEASE_OUTSIDE:
		_mouseState = MOUSE_UP;
		flag |= (ButtonAction::IDLE_TO_OVER_UP);
		break;
	default:
		//else if (id.m_id == event_id::DRAG_OUT) c |= (button_action::OVER_DOWN_TO_OUT_DOWN);
		//else if (id.m_id == event_id::DRAG_OVER) c |= (button_action::OUT_DOWN_TO_OVER_DOWN);
		//else if (id.m_id == event_id::RELEASE_OUTSIDE) c |= (button_action::OUT_DOWN_TO_IDLE);
		break;
	}
	// setup display list
	setupFrame();

#ifdef SWF_DEBUG
	const char*strEvent[] = {"N/A","PRESS","RELEASE","RELEASE_OUTSIDE","ROLL_OVER","ROLL_OUT","DRAG_OVER","DRAG_OUT","KEY_PRESS"};
	MK_TRACE("event:%s on %x\n",strEvent[code],this);
#endif

	// actions can delete THIS & PARENT through execute_frame_tags()
	ButtonActionArray::iterator it = getDefinition()._buttonActions.begin();
	while( getDefinition()._buttonActions.end() != it ) {
		if ((*it)->_conditions & flag) {
			(*it)->_actions.setup(_parent);
		}
		++it;
	}
}

	
}//namespace