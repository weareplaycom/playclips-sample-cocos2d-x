#include <iostream>
#include <set>
#include <stdio.h>
#include "SimpleAudioEngine.h"

#include "Adjust/Adjust2dx.h"

#include "json/rapidjson.h"
#include "json/document-wrapper.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "network/HttpClient.h"
#include "ui/UIVideoPlayer.h"
#include "extensions/assets-manager/AssetsManagerEx.h"
#include "extensions/assets-manager/CCEventListenerAssetsManagerEx.h"

#include "PlayClipsSampleScene.h"
#include "PlayClipsModels.h"

USING_NS_CC;
USING_NS_CC_EXT;

using namespace rapidjson;
using namespace network;

PlayClipsSample* privateInstance;

// Helper to iterate through a container (C) elements,
// call a predicate (P) and keep the results in
// another container (R)
template<typename R, typename C, typename P>
void mapWithIndex(R& r, const C& c, P pred) {
    int idx = 0;
    for (auto& elem : c) {
        r.pushBack(pred(elem, idx));
        idx++;
    }
}

// Helper to iterate through a container (C) elements,
// call a predicate (P)
template<typename C, typename P>
void forEach(const C& c, P pred) {
    for (auto& elem : c) {
        pred(elem);
    }
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    cocos2d::log("Error while loading: %s", filename);
    cocos2d::log("Depending on how you compiled you might have to add 'Resources/' in front of filenames in PlayClipsSampleScene.cpp");
}

Scene* PlayClipsSample::createScene()
{
    // We need this privateInstance for the deferredDeeplinkCallbackMethod
    // as we cannot use std::function and std::bind
    privateInstance = PlayClipsSample::create();
    return privateInstance;
}

// Obtain the influencer identifier from user preferences, previously
// stored via the deferred deeplink flow
static std::string loadInfluencerId() {
    return UserDefault::getInstance()->getStringForKey("playclips.influencer");
}

// Save the influencer identifier in user preferences
static void saveInfluencerId(std::string influencer_id) {
    UserDefault::getInstance()->setStringForKey("playclips.influencer", influencer_id);
}

// Static method to obtain the deferred deeplink information from
// Adjust and extract the influencer
bool PlayClipsSample::deferredDeeplinkCallbackMethod(std::string deeplink) {
    cocos2d::log("\nDeferred deep link received!");

    // Example of deeplink as sent by Adjust:
    //      plsample://welcomeback?influencer=Wally&adjust_no_sdkclick=1
    cocos2d::log("\nURL: %s\n", deeplink.c_str());

    // Notify Adjust
    Adjust2dx::appWillOpenUrl(deeplink);

    // Parse deeplink information
    DeferredDeeplink playClipsDeeplink(deeplink);

    auto component = (MenuItemFont*)privateInstance->menuStart->getChildByName("waiting");

    if (playClipsDeeplink.isValid()) {
        // TODO: send an InApp event to track the success
        cocos2d::log("Influencer is: %s", playClipsDeeplink.getInfluencer().c_str());

        saveInfluencerId(playClipsDeeplink.getInfluencer());
        component->setString("The influencer \nassigned to you is:\n\n" +
                             playClipsDeeplink.getInfluencer() +
                             "\n\nClick to start!");

        // Launch the influencer catalog download
        privateInstance->downloadZipCatalog(playClipsDeeplink.getInfluencer());
        return true;
    } else {
        cocos2d::log("Error: Expected influencer but not received");
        // TODO: send an InApp event to track the error
        return false;
    }
}

// Initialize the instance
bool PlayClipsSample::init()
{

    // A Scene is a container that holds Sprites, Labels, Nodes and other objects that your game needs.
    // A Scene is responsible for running game logic and rendering the content on a per-frame basis.
    
    if (!Scene::init()) {
        return false;
    }
    
    // Director controls every aspect of your game:
    //   - what is shown on the screen
    //   - what sounds are played
    //   - what happens with player input, and much more.

    auto origin      = Director::getInstance()->getVisibleOrigin();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // Create bottom menu: Play video, Quit program
    // CC_CALLBACK_1 binds the method PlayClipsSample::menuCloseCallback to this instance
    // and explicits that 1 parameter is expected by the function
    auto closeItem = MenuItemImage::create("CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(PlayClipsSample::menuCloseCallback, this));
    
    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0) {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    } else {
        closeItem->setAnchorPoint(Vec2(1,0));   // bottom-right point
        auto x = origin.x + visibleSize.width;  // x: end of the visible size
        auto y = origin.y;                      // y: bottom
        closeItem->setPosition(Vec2(x,y));
    }
    
    // create bottom menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);

    this->addChild(menu, 1);
    
    // Start menu item
    auto influencerItem = MenuItemFont::create("Waiting for influencer...",
                                            CC_CALLBACK_1(PlayClipsSample::loadInfluencerMetadata, this));
    
    influencerItem->setAnchorPoint(Vec2(0.5,0.5));
    influencerItem->setFontNameObj("Arial");
    influencerItem->setFontSizeObj(20);
    influencerItem->setPosition(Vec2(0, 0));
    influencerItem->setName("waiting");
    
    // create start menu, it's an autorelease object
    menuStart = Menu::create(influencerItem, NULL);
    menuStart->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    this->addChild(menuStart, 1);
    
    std::string influencer = loadInfluencerId();
    
    if (!influencer.empty()) {
        auto component = (MenuItemFont*)this->menuStart->getChildByName("waiting");
        component->setString("The influencer \nassigned to you is:\n\n" + influencer + "\n\nClick to start!");
    }

    // add "PlayClips" icon
    // a sprite was a bitmap image with hardware support to be movable.
    // Early game hardware could handle a few sprites, often used to represent the player and enemies in the world.
    // In real world examples, in Super Mario Brothers, Mario, the mushrooms, coins and such would be sprites.
    // These days, “Sprite” is basically a bitmap image (or portion of a bitmap, we’ll see this later)
    // along with positional information.  These days the concept of hardware sprites doesn’t really exist anymore.
    sprite = Sprite::create("playclips_400.png");

    if (sprite == nullptr) {
        problemLoading("'playclips_400.png'");
    } else {
        sprite->setAnchorPoint(Vec2(0.5, 1));
        sprite->setScale(0.5f);
        sprite->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height * 0.95));
        
        // Add "touch" event listener to the sprite
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->onTouchBegan = [](Touch* touch, Event* event) -> bool {
            auto bounds = event->getCurrentTarget()->getBoundingBox();
            
            if (bounds.containsPoint(touch->getLocation())){
                std::stringstream touchDetails;
                touchDetails
                    << "This is a PlayClips sample demo, brought to you with love by PLayClips team"
                    << std::endl;
                MessageBox(touchDetails.str().c_str(), "About");
            }
            return true;
        };
        Director::getInstance()
            ->getEventDispatcher()
            ->addEventListenerWithSceneGraphPriority(touchListener, sprite);

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }
    
    this->scheduleUpdate();
    // Relative priority: in case the node you are updating doesn’t actually need to be updated every frame,
    // to prevent wasting CPU power ( and battery life
    // this->scheduleUpdateWithPriority(100);
    
    // Schedule updates by defining the method to be called instead of overriding update method
    //this->schedule(CC_CALLBACK_1(PlayClipsSample::update, this), "foo");
    
    //auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    //audio->preloadBackgroundMusic("demo.mp3");
    //audio->playBackgroundMusic("demo.mp3");
    return true;
}

// Create the influencer catalog file that will be used by AssetManagerEx
// to download the catalog
std::string generateInfluencerFile(std::string influencer) {
    FileUtils *fileUtils = FileUtils::getInstance();
    std::string path = FileUtils::getInstance()->getWritablePath() + influencer + ".catalog";

    // Update the default playclips.catalog file with the influencer identifier obtained
    // via deferred deeplink
    if (fileUtils->isFileExist("playclips.catalog")) {
        const std::string placeholder = "{influencer}";

        rapidjson::Document jsonCatalogData;
        std::string data = fileUtils->getStringFromFile("playclips.catalog");
        jsonCatalogData.Parse<kParseDefaultFlags>(data.c_str());

        // Update the remote manifest URL
        rapidjson::Value& manifestUrl = jsonCatalogData["remoteManifestUrl"];
        std::string manifestValue = manifestUrl.GetString();
        manifestValue = manifestValue.replace(manifestValue.find(placeholder), placeholder.length(), influencer);
        manifestUrl.SetString(manifestValue.c_str(), manifestValue.length());

        // Update the remote version URL
        rapidjson::Value& versionUrl = jsonCatalogData["remoteVersionUrl"];
        std::string versionValue = versionUrl.GetString();
        versionValue = versionValue.replace(versionValue.find(placeholder), placeholder.length(), influencer);
        versionUrl.SetString(versionValue.c_str(), versionValue.length());

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        jsonCatalogData.Accept(writer);

        FILE *fp = fopen(path.c_str(), "w");
        if (!fp) {
            cocos2d::log("can not create file %s", path.c_str());
        } else {
            // Store the JSON content in the new file
            fputs(buffer.GetString(), fp);
            fclose(fp);
            cocos2d::log("File created: %s", path.c_str());
        }
    } else {
        cocos2d::log("Missing playclips.catalog template file");
    }
    return path;
}

void PlayClipsSample::downloadZipCatalog(std::string influencer)
{
    // TODO: handle errors while creating the file

    // Create a file compatible with AssetManagerEx format
    // http://www.cocos2d-x.org/wiki/Assets_Manager_Extension
    std::string influencerCatalog = generateInfluencerFile(influencer);
    std::string storagePath = FileUtils::getInstance()->getWritablePath() + "playclips/" + influencer;
    cocos2d::log("Storage path for the catalog: %s", storagePath.c_str());

    // Launch the influencer asset catalog download
    AssetsManagerEx* _am = AssetsManagerEx::create(influencerCatalog, storagePath);
    
    _am->retain();
    
    if (!_am->getLocalManifest()->isLoaded()) {
        cocos2d::log("Fail to update assets, step skipped.");
    } else {
        EventListenerAssetsManagerEx* _amListener = EventListenerAssetsManagerEx::create(_am, [&_am, this](EventAssetsManagerEx* event) {
            static int failCount = 0;
            
            switch (event->getEventCode()) {
                case EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST:
                    cocos2d::log("No local manifest file found, skip assets update.");
                    break;
                case EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION: {
                    std::string assetId = event->getAssetId();
                    float percent = event->getPercent();
                    std::string str;
                    if (assetId == AssetsManagerEx::VERSION_ID) {
                        str = StringUtils::format("Version file: %.2f", percent) + "%";
                    } else if (assetId == AssetsManagerEx::MANIFEST_ID) {
                        str = StringUtils::format("Manifest file: %.2f", percent) + "%";
                    } else {
                        str = StringUtils::format("%.2f", percent) + "%";
                        cocos2d::log("%.2f Percent", percent);
                    }
                    break;
                }
                case EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST:
                case EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST:
                    cocos2d::log("Fail to download manifest file, update skipped.");
                    break;
                case EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE:
                case EventAssetsManagerEx::EventCode::UPDATE_FINISHED: {
                    cocos2d::log("Update finished. %s", event->getMessage().c_str());
                    break;
                }
                case EventAssetsManagerEx::EventCode::UPDATE_FAILED:
                    cocos2d::log("Update failed. %s", event->getMessage().c_str());
                    failCount ++;
                    if (failCount < 5) {
                        _am->downloadFailedAssets();
                    } else {
                        CCLOG("Reach maximum fail count, exit update process");
                        failCount = 0;
                    }
                    break;
                case EventAssetsManagerEx::EventCode::ERROR_UPDATING:
                    cocos2d::log("Asset %s : %s", event->getAssetId().c_str(), event->getMessage().c_str());
                    break;
                case EventAssetsManagerEx::EventCode::ERROR_DECOMPRESS:
                    cocos2d::log("%s", event->getMessage().c_str());
                    break;
                default:
                    break;
            }
        });
        Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_amListener, 1);
        _am->update();
    }
}

 
void PlayClipsSample::update(float delta) {
}


void PlayClipsSample::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void PlayClipsSample::influencerSelected() {
    Vector<MenuItem*> tagsMenu;
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::set<std::string> tags;
    
    forEach(this->influencer->getVideos(), [&tags](Video* video) {
        forEach(video->getTags(), [&tags](std::string tag){
            tags.insert(tag);
        });
    });
    
    cocos2d::log("Tags set size is %lu", tags.size());
    
    mapWithIndex(tagsMenu, tags, [&origin, this](std::string tag, int idx) {
        auto item = MenuItemFont::create(tag,
                                         CC_CALLBACK_1(PlayClipsSample::playVideo,
                                                       this,
                                                       tag));
        item->setName(tag);
        item->setFontNameObj("Arial");
        item->setFontSizeObj(14);
        item->setAnchorPoint(Vec2(0, 1));
        item->setPosition(Vec2(origin.x, origin.y - 30*idx-10));
        return item;
    });

    auto menu = Menu::createWithArray(tagsMenu);
    menu->setAnchorPoint(Vec2(0, 1));
    menu->setPosition(Vec2(origin.x + 10, sprite->getPosition().y - sprite->getContentSize().height - 10));
    
    this->addChild(menu, 1);
    
}

void PlayClipsSample::playVideo(Ref* pSender, std::string tag) {

    Video* video = this->influencer->getVideoByTag(tag);
    std::string str = video->getLocation();
    str = FileUtils::getInstance()->getWritablePath()  + "playclips/" + this->influencer->getId() + "/" + str;

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    cocos2d::log("Play Video %s", str.c_str());

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    cocos2d::experimental::ui::VideoPlayer* videoPlayer = cocos2d::experimental::ui::VideoPlayer::create();
    videoPlayer->setContentSize(cocos2d::Size(160, 120));

    float x = origin.x + visibleSize.width;
    float y = origin.y + visibleSize.height;
    
    videoPlayer->setAnchorPoint(Vec2(1, 1));  // top-right point
    videoPlayer->setPosition(Vec2(x,y));
    
    videoPlayer->setFileName(str);
    videoPlayer->setVisible(true);
    videoPlayer->setFullScreenEnabled(false);

    videoPlayer->addEventListener([this](Ref* pointer, cocos2d::experimental::ui::VideoPlayer::EventType eventType) {
        cocos2d::log("Received event: %d", eventType);
        switch (eventType) {
            case cocos2d::experimental::ui::VideoPlayer::EventType::COMPLETED: {
                cocos2d::log("Delete player");
                cocos2d::experimental::ui::VideoPlayer* player = (cocos2d::experimental::ui::VideoPlayer *)pointer;
                player->onExit();
                break;
            }
            default:
                break;
        }
    });

    this->addChild(videoPlayer, 10, 1);
    videoPlayer->play();
#else
    std::string unsupportedPlatform = "I was about to reproduce "+video->getId()+", but your platform does not support Video player";
    cocos2d::log("%s", unsupportedPlatform.c_str());
    MessageBox(unsupportedPlatform.c_str(), "About");
#endif
    
}

void PlayClipsSample::loadInfluencerMetadata(Ref* pSender)
{
    menuStart->setVisible(false);
    std::string influencer_id = loadInfluencerId();
    cocos2d::log("Loading metadadata from disk for influencer: %s", influencer_id.c_str());
    std::string influencerCatalogFile = FileUtils::getInstance()->getWritablePath()  +
                                        "playclips/" + influencer_id + "/metadata.json";
    
    // Load and parse metadata.json content
    const char * data = FileUtils::getInstance()->getStringFromFile(influencerCatalogFile).c_str();
    jsonCatalog.Parse<kParseDefaultFlags>(data);

    Influencer* inf = new Influencer(influencer_id,
                                     jsonCatalog["name"].GetString(),
                                     jsonCatalog["thumbnail"].GetString());

    // Load video info (local relative location, tags)
    for (rapidjson::Value::ConstMemberIterator itr = jsonCatalog["videos"].MemberBegin();
         itr != jsonCatalog["videos"].MemberEnd();
         ++itr) {

        Video* video = new Video(itr->name.GetString(),
                                 itr->value["location"].GetString(),
                                 itr->value["weight"].GetInt());

        auto tags = itr->value["tags"].GetArray();

        for (auto& tag:tags) {
            cocos2d::log("Adding tag %s to the list of the video %s for the influencer %s",
                         tag.GetString(),
                         video->getId().c_str(),
                         inf->getId().c_str());
            video->addTag(tag.GetString());
        }
        inf->addVideo(video);
    }
    this->influencer = inf;

    this->influencerSelected();
}
