#import <MetalKit/MetalKit.h>

@interface Renderer : NSObject <MTKViewDelegate>

-(nonnull instancetype)initWithView:(nonnull MTKView *)view;

-(void)handleEvent:(nonnull NSEvent*)event;
-(void)handleMouseButtonEvent:(nonnull NSEvent*)event;
-(void)handleMouseScrollEvent:(nonnull NSEvent*)event;

@end

