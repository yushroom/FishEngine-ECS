#import "ViewController.h"
#import <MetalKit/MetalKit.h>
#include "Renderer.h"
#include <imgui.h>
#include <imgui_impl_osx.h>

@interface ViewController()
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) Renderer *renderer;
@end

@implementation ViewController

- (MTKView *)mtkView {
	return (MTKView *)self.view;
}

- (void)viewDidLoad {
	[super viewDidLoad];
	
	self.mtkView.device = MTLCreateSystemDefaultDevice();
	
	if (!self.mtkView.device) {
		NSLog(@"Metal is not supported");
		abort();
	}
	
	self.renderer = [[Renderer alloc] initWithView:self.mtkView];
	
//	[self.renderer mtkView:self.mtkView drawableSizeWillChange:self.mtkView.bounds.size];

	self.mtkView.delegate = self.renderer;

	// Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
	NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
																options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
																  owner:self
															   userInfo:nil];
	[self.view addTrackingArea:trackingArea];
	
#if 0
	// If we want to receive key events, we either need to be in the responder chain of the key view,
	// or else we can install a local monitor. The consequence of this heavy-handed approach is that
	// we receive events for all controls, not just Dear ImGui widgets. If we had native controls in our
	// window, we'd want to be much more careful than just ingesting the complete event stream, though we
	// do make an effort to be good citizens by passing along events when Dear ImGui doesn't want to capture.
	NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged | NSEventTypeScrollWheel;
	[NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent *event) {
		BOOL wantsCapture = ImGui_ImplOSX_HandleEvent(event, self.view);
		if (event.type == NSEventTypeKeyDown && wantsCapture) {
			return nil;
		} else {
			return event;
		}
	}];
#endif
	
	ImGui_ImplOSX_Init();
	
//	[self.mtkView setPreferredFramesPerSecond:30];
}

- (void)viewDidAppear
{
	// Make the view controller the window's first responder so that it can handle the Key events
	[self.mtkView.window makeFirstResponder:self];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}


- (void)setRepresentedObject:(id)representedObject {
	[super setRepresentedObject:representedObject];

	// Update the view, if already loaded.
}

- (void)keyDown:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleKeyEvent:event];
}

- (void)keyUp:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleKeyEvent:event];
}

- (void)mouseMoved:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleEvent:event];
}

- (void)mouseDown:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleMouseButtonEvent:event];
}

- (void)mouseUp:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleMouseButtonEvent:event];
}

- (void)mouseDragged:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleEvent:event];
}

- (void)scrollWheel:(NSEvent *)event {
	ImGui_ImplOSX_HandleEvent(event, self.view);
	[self.renderer handleMouseScrollEvent:event];
}

@end
