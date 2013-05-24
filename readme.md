MonkSWF -- An SWF Animation Player 
==================================

## Overview

MonkSWF is a SWF (Adobe Flash) animation player.  It uses OpenVG as the rendering API. 
If your platform does not support OpenVG you can use [MonkVG](https://github.com/micahpearlman/MonkVG).

If you are looking to integrate this with your Cocos2D application there is an excellent article by Luke. 
Check it out: http://blog.zincroe.com/2011/11/displaying-a-swf-on-the-iphone-with-cocos2d-and-monkswf/

In OpenGL edition, MonkSWF is modified for bitmap animation and is not dependent on OpenVG, therefore,
vector rendering is not supported. It also provide features for clipping, audio event and export bitmap filename by assets.


## Installation

Use git to clone:

<tt>$ git clone git@github.com:micahpearlman/MonkSWF.git</tt>

If your platform does not support OpenVG download MonkVG with git:

<tt>$ git clone git@github.com:micahpearlman/MonkVG.git</tt>

TODO: standard *nix Makefiles.



## What is implemented

- Sprite animations.

## TODO 

- DoAction tags
- Most basic solid fills and strokes.
- Bitmap fills and strokes. 
- Gradient fills and strokes.
- Font Rendering.
- Morph Targets.

## Simple Example

NOTE:  MonkVG will not create a OpenGL context, it is the applications
responsibility to create there own OpenGL context. Also, if your
application does any other OpenGL rendering it should save off the GL
state and then restore before calling any MonkVG or MonkSWF methods.


Graphics and Audio handling:

	render handler
        virtual unsigned int getTexture( const char *filename ) = 0;
        virtual void applyTexture( unsigned int texture ) = 0;
        virtual void applyTransform( const MATRIX3f& mtx ) = 0;
        virtual void drawQuad(const RECT& rect, const CXFORM&) = 0;
        virtual void maskBegin(void) = 0;
        virtual void maskEnd(void) = 0;
        virtual void maskOffBegin(void) = 0;
        virtual void maskOffEnd(void) = 0;

	sound handler
        virtual unsigned int getSound( const char *filename ) = 0;
        virtual void playSound( unsigned int sound, bool stop, bool noMultiple, bool loop ) = 0;

	asset handler
        typedef uint32_t (*LoadAssetCallback)( const char *name );
		
<pre><code>
	uint32_t loadAssetCallback( const char *name )
	{
		if (strstr(name,".png"))
		{
			return MonkSWF::Renderer::getRenderer()->getTexture(name);
		}
		else if (strstr(name,".wav"))
		{
			return MonkSWF::Speaker::getSpeaker()->getSound(name);
		}
		return 0;
	}
</code></pre>


Initialization:

<pre><code>

	#include <MonkSWF.h>
	
	char* filebuffer;
	int filebuffersize;
	... Load The SWF File into a buffer ...
	MonkSWF::Reader r( filebuffer, filebuffersize );
    MonkSWF::Renderer::setRenderer(new glRenderer);
    MonkSWF::Speaker::setSpeaker(new fmodSpeaker);
    MonkSWF::SWF::initialize(loadAssetCallback);
	MonkSWF::SWF* swf = new SWF();
	swf->read( &r );	// read in SWF file

</code></pre>

Rendering and Animation

This example shows rendering the root SWF Movie.

<pre><code>

	... begin opengl context ...
	
	float deltaTime = calculateDeltaTime();
	swf->update(deltaTime);

	swf->draw();

	... end opengl context ...
</code></pre>

