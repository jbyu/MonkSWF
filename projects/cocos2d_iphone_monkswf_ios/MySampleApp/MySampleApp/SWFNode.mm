#include <vg/openvg.h>
#include <vg/vgu.h>
#import "SWFNode.h"


@interface SWFNode ()

@property (nonatomic, readwrite, assign) CFTimeInterval elapsed;

@property (nonatomic, readwrite, assign) NSUInteger frame;

@property (nonatomic, readwrite, assign) MonkSWF::SWF *swf;


@end



@implementation SWFNode


#pragma mark -

+ (void)initialize {
    ccDirectorProjection projection = [[CCDirector sharedDirector] projection];
    vgCreateContextSH(0, 0);
    [[CCDirector sharedDirector] setProjection:projection];
}

+ (id)nodeWithFile:(NSString *)filename {
    return [[[self alloc] initWithFile:filename] autorelease];
}


#pragma mark -

- (id)initWithFile:(NSString *)filename {
    if (self = [super init]) {
        
        [self setAnchorPoint:ccp(0.5, 0.5)];
        
        NSString *path;
        path = [[NSBundle mainBundle] pathForResource:filename ofType:nil];
        
        NSData *data;
        data = [NSData dataWithContentsOfFile:path];
        
        int length = [data length];
        char *buffer = new char[length];
        
        // copy the data into the buffer
        [data getBytes:buffer length:length];
        
        // create a reader using the buffer
        MonkSWF::Reader reader(buffer, length);
        
        // create the swf using the reader
        swf_ = new MonkSWF::SWF();
        swf_->initialize();
        swf_->read(&reader);
        
        // clean up the buffer
        delete [] buffer;
        
    }
    
    return self;
}

- (void)dealloc {
    delete swf_;
    [super dealloc];
}


#pragma mark -

@synthesize elapsed = elapsed_;

@synthesize frame = frame_;

@synthesize swf = swf_;


#pragma mark -

- (void)onEnter {
    [super onEnter];
    [self scheduleUpdate];
}

- (void)update:(ccTime)delta {
    
    CFTimeInterval interval;
    interval = 1.0f / self.swf->getFrameRate();
    
    NSUInteger total;
    total = self.swf->numFrames();
    
    self.elapsed += delta;
    
    while (self.elapsed > interval) {
        self.elapsed -= interval;
        self.frame++;
        if (self.frame >= total) {
            self.frame = 0;
        }
    }

}

- (void)onExit {
    [self unscheduleUpdate];
    [super onExit];
}


#pragma mark -

- (void)draw {
    
    CCDirector *director;
    director = [CCDirector sharedDirector];
    
    ccDirectorProjection projection;
    projection = director.projection;
    [director setProjection:kCCDirectorProjection2D];
    
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glPushMatrix();
    
    vgLoadIdentity();
    vgTranslate(self.swf->getFrameWidth() * self.anchorPoint.x * -1.0f, self.swf->getFrameHeight() * self.anchorPoint.y * -1.0f);
    vgScale(self.scaleX * CC_CONTENT_SCALE_FACTOR() * 1.0f, self.scaleY * CC_CONTENT_SCALE_FACTOR() * -1.0f);
    vgRotate(self.rotation * -1.0f);
    vgTranslate(self.positionInPixels.x, self.positionInPixels.y);
	self.swf->drawFrame(self.frame);
    
    glPopMatrix();
    
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    [director setProjection:projection];
    
}


@end
