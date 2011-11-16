#import "AppDelegate.h"
#import "RootViewController.h"
#import "SWFNode.h"

@implementation AppDelegate


- (void)dealloc {
	[[CCDirector sharedDirector] end];
	[window_ release];
	[super dealloc];
}


@synthesize window = window_;


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	
    
    // cocos2d
    
	[CCDirector setDirectorType:kCCDirectorTypeDisplayLink];
	[CCTexture2D setDefaultAlphaPixelFormat:kCCTexture2DPixelFormat_RGBA8888];
    
	EAGLView *openGLView;
    openGLView = [EAGLView viewWithFrame:[[UIScreen mainScreen] bounds]
                             pixelFormat:kEAGLColorFormatRGBA8
                             depthFormat:0
                      preserveBackbuffer:NO
                              sharegroup:nil
                           multiSampling:YES
                         numberOfSamples:1];
    
    CCDirector *director;
    director = [CCDirector sharedDirector];
	[director setAnimationInterval:1.0/60.0];
	[director setDeviceOrientation:kCCDeviceOrientationPortrait];
	[director setDisplayFPS:YES];
	[director setOpenGLView:openGLView];
	[director enableRetinaDisplay:YES];
    
    
    // uikit
    
    UIViewController *viewController;
    viewController = [[[RootViewController alloc] initWithNibName:nil bundle:nil] autorelease];
    [viewController setWantsFullScreenLayout:YES];
    [viewController setView:openGLView];
    
    UIWindow *window;
    window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    [window setRootViewController:viewController];
    [window makeKeyAndVisible];
    [self setWindow:window];
    
    
    // app
    
    CCScene *scene;
    scene = [CCScene node];
    [director runWithScene:scene];
    
    CCLayer *layer;
    layer = [CCLayer node];
    [scene addChild:layer];
    
    SWFNode *puppy;
    puppy = [SWFNode nodeWithFile:@"puppy.swf"];
    [puppy setPosition:ccp(240, 160)];
    [layer addChild:puppy];
    
    return YES;
    
}

- (void)applicationWillResignActive:(UIApplication *)application {
	[[CCDirector sharedDirector] pause];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	[[CCDirector sharedDirector] resume];
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
	[[CCDirector sharedDirector] purgeCachedData];
}

-(void) applicationDidEnterBackground:(UIApplication*)application {
	[[CCDirector sharedDirector] stopAnimation];
}

-(void) applicationWillEnterForeground:(UIApplication*)application {
	[[CCDirector sharedDirector] startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application {
	[[[CCDirector sharedDirector] openGLView] removeFromSuperview];
	[[CCDirector sharedDirector] end];
}

- (void)applicationSignificantTimeChange:(UIApplication *)application {
	[[CCDirector sharedDirector] setNextDeltaTimeZero:YES];
}


@end
