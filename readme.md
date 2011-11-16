MonkSWF -- An SWF Animation Player ==================================

## Overview

MonkSWF is a SWF (Adobe Flash) animation player.  It uses OpenVG as the
rendering API.  If your platform does not support OpenVG you can use [MonkVG](https://github.com/micahpearlman/MonkVG).

If you are looking to integrate this with your Cocos2D application there is an excellent article by
Luke.  Check it out: http://blog.zincroe.com/2011/11/displaying-a-swf-on-the-iphone-with-cocos2d-and-monkswf/

## Installation

Use git to clone:

<tt>$ git clone git@github.com:micahpearlman/MonkSWF.git</tt>

If your platform does not support OpenVG download MonkVG with git:

<tt>$ git clone git@github.com:micahpearlman/MonkVG.git</tt>

There are currently iOS projects.

TODO: standard *nix Makefiles.



## What is implemented

- Most basic solid fills and strokes.
- Sprite animations.

## TODO 

- Bitmap fills and strokes. 
- Gradient fills and strokes.
- Font Rendering.
- Morph Targets.

## Simple Example

NOTE:  MonkVG will not create a OpenGL context, it is the applications
responsibility to create there own OpenGL context. Also, if your
application does any other OpenGL rendering it should save off the GL
state and then restore before calling any MonkVG or MonkSWF methods.

Initialization:

<tt>
#include <MonkSWF.h>
#include <vg/openvg.h>
#include <vg/vgu.h>

char* filebuffer;
int filebuffersize;
... Load The SWF File into a buffer ...
MonkSWF::Reader r( filebuffer, filebuffersize );
		
MonkSWF::SWF* swf = new SWF();
swf->initialize();
swf->read( &r );	// read in SWF file

</tt>

Rendering and Animation

This example shows rendering the root SWF Movie.

<tt>

	... begin opengl context ...
	
	float deltaTime = calculateDeltaTime();
	if ( deltaTime > 1.0f / swf->getFrameRate() ) {
		frame++;
	}
	if ( frame >= swf->numFrames() ) {	// loop
		frame = 0;
	}
	
	// setup OpenVG
	vgLoadIdentity();
	vgRotate( angle );
	
	swf->drawFrame( frame );

	... end opengl context ...
</tt>

Alternatively you render a specific sprite in a SWF.

<tt>
	// get a specific sprite
	MonkSWF::IDefineSpriteTag*	sprite = swf->spriteAt( spriteIdx );
	
	... begin opengl context ...
	
	float deltaTime = calculateDeltaTime();
	if ( deltaTime > 1.0f / swf->getFrameRate() ) {
		frame++;
	}
	if ( frame >= sprite->frameCount() ) {	// loop
		frame = 0;
	}
	
	// setup OpenVG
	vgLoadIdentity();
	vgRotate( angle );
	
	sprite->draw( frame );

	... end opengl context ...

</tt>