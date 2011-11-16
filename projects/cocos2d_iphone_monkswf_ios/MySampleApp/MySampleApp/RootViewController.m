#import "RootViewController.h"


@implementation RootViewController


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return UIInterfaceOrientationIsLandscape(interfaceOrientation);
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration {

	CGRect rect;
    rect = [[UIScreen mainScreen] bounds];
    
    if (UIInterfaceOrientationIsLandscape(interfaceOrientation)) {
        rect.size = CGSizeMake(rect.size.height, rect.size.width);
    }
    
	[[[CCDirector sharedDirector] openGLView] setFrame:rect];
    
}


@end

