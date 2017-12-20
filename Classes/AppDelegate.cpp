#include "AppDelegate.h"

#include "Adjust/Adjust2dx.h"
#include "PlayClipsSampleConfig.h"
#include "PlayClipsSampleScene.h"

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(320, 480);
static cocos2d::Size smallResolutionSize = cocos2d::Size(320, 480);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(768, 1024);
static cocos2d::Size largeResolutionSize = cocos2d::Size(1536, 2048);

AppDelegate::AppDelegate() {
}

AppDelegate::~AppDelegate() {
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs() {
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages() {
    return 0; //flag for packages manager
}

#define CC_TARGET_PLATFORM_PC CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX

bool AppDelegate::applicationDidFinishLaunching() {
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    
    if(!glview) {
#if CC_TARGET_PLATFORM_PC
        glview = GLViewImpl::createWithRect("PlayClips demo",
                                            cocos2d::Rect(0, 0,
                                                          designResolutionSize.width,
                                                          designResolutionSize.height));
#else
        glview = GLViewImpl::create("PlayClips demo");
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    //director->setDisplayStats(true);
    
    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width,
                                    designResolutionSize.height,
                                    ResolutionPolicy::NO_BORDER);
    
//    auto frameSize = glview->getFrameSize();
    
    // if the frame's height is larger than the height of medium size.
//    if (frameSize.height > mediumResolutionSize.height) {
//        director->setContentScaleFactor(
//            MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width)
//        );
//    }
//    // if the frame's height is larger than the height of small size.
//    else if (frameSize.height > smallResolutionSize.height) {
//        director->setContentScaleFactor(
//            MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width)
//        );
//    }
//    // if the frame's height is smaller than the height of medium size.
//    else {
//        director->setContentScaleFactor(
//            MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width)
//        );
//    }

    register_all_packages();

    // it's an autorelease object
    auto scene = PlayClipsSample::createScene();

    // Initialize Adjust SDK
    AdjustConfig2dx adjustConfig = AdjustConfig2dx(PlayClipsSampleConfig::adjustAppToken,
                                                   PlayClipsSampleConfig::adjustEnvironment);

    adjustConfig.setLogLevel(PlayClipsSampleConfig::adjustLogLevel);

    adjustConfig.setDeferredDeeplinkCallback(PlayClipsSample::deferredDeeplinkCallbackMethod);
    cocos2d::log("Starting Adjust SDK");

    Adjust2dx::start(adjustConfig);

    // Launch the main page
    director->runWithScene(scene);

    return true;
}

// This function will be called from native code to continue an user web activity (deeplink)
bool AppDelegate::openURL(const std::string &url) {
    cocos2d::log("Open Url starts with url value: %s", url.c_str());
    PlayClipsSample::deferredDeeplinkCallbackMethod(url);
    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    cocos2d::log("Background starts");
    Director::getInstance()->stopAnimation();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    cocos2d::log("Foreground starts");
    Director::getInstance()->startAnimation();
}
