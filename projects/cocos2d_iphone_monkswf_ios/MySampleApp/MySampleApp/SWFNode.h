#import <Foundation/Foundation.h>
#import "cocos2d.h"
#import "MonkSWF.h"

@interface SWFNode : CCNode {
    CFTimeInterval elapsed_;
    NSUInteger frame_;
    MonkSWF::SWF *swf_;
}

+ (id)nodeWithFile:(NSString *)filename;

- (id)initWithFile:(NSString *)filename;

@end
