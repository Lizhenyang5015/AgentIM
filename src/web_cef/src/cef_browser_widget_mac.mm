#import <AppKit/AppKit.h>

namespace agentim::web {

namespace {

NSView* browserView(void* view_handle) {
    return static_cast<NSView*>(view_handle);
}

}  // namespace

void resizeMacBrowserView(void* view_handle, int width, int height) {
    NSView* view = browserView(view_handle);
    if (view == nil) {
        return;
    }

    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    view.frame = NSMakeRect(0.0, 0.0, width, height);
    view.needsDisplay = YES;
}

}  // namespace agentim::web
