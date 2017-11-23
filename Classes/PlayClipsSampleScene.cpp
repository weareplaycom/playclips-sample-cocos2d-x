#include <iostream>
#include <set>
#include "SimpleAudioEngine.h"
#include "json/rapidjson.h"
#include "json/document-wrapper.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "network/HttpClient.h"
#include "ui/UIVideoPlayer.h"
#include "PlayClipsSampleScene.h"
#include "PlayClipsModels.h"

using namespace cocos2d;
using namespace rapidjson;
using namespace network;

#define CONNECT_TO_CDN 1

Scene* PlayClipsSample::createScene()
{
    return PlayClipsSample::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    cocos2d::log("Error while loading: %s", filename);
    cocos2d::log("Depending on how you compiled you might have to add 'Resources/' in front of filenames in PlayClipsSampleScene.cpp");
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
    auto loadCatalog = MenuItemFont::create("Press here to start",
                                            CC_CALLBACK_1(PlayClipsSample::loadCatalog, this));
    
    loadCatalog->setAnchorPoint(Vec2(0.5,0.5));
    loadCatalog->setFontNameObj("Arial");
    loadCatalog->setFontSizeObj(20);
    loadCatalog->setPosition(Vec2(0, 0));

    // create start menu, it's an autorelease object
    menuStart = Menu::create(loadCatalog, NULL);
    menuStart->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    this->addChild(menuStart, 1);

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

 
void PlayClipsSample::update(float delta) {
    // delta: amount of time, in seconds since the last time the update function was called
//    auto position = sprite->getPosition();
//    auto new_position = position.x - 100 * delta;
//    position.x = new_position >= 0 - sprite->getBoundingBox().size.width / 2
//               ? new_position
//               : this->getBoundingBox().getMaxX() + sprite->getBoundingBox().size.width/2;
//    sprite->setPosition(position);
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

void PlayClipsSample::onHttpRequestCatalog(network::HttpClient *sender, network::HttpResponse *response) {
    
    if (response && response->getResponseCode() == 200 && response->getResponseData()) {
        std::vector<char> *data = response->getResponseData();
        // Extract information from JSON payload
        loadJsonCatalog(data->data());
    }
    else {
        cocos2d::log("Error while loading the catalog: %ld", response->getResponseCode());
    }
}

void PlayClipsSample::loadJsonCatalog(const char* data) {
    jsonCatalog.Parse<kParseDefaultFlags>(data);
    
    typedef GenericDocument<UTF8<>, MemoryPoolAllocator<>, CrtAllocator>::ValueType InfluencerData;
    
    Vector<MenuItem*> influencerNames;
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    const char* inf_id;
    
    for (rapidjson::Value::ConstMemberIterator itr = jsonCatalog.MemberBegin(); itr != jsonCatalog.MemberEnd(); ++itr) {
        cocos2d::log("Loaded influencer %s with name %s",
                     itr->name.GetString(),
                     jsonCatalog[itr->name.GetString()]["name"].GetString());
        
        inf_id = itr->name.GetString();
        
        Influencer* inf = new Influencer(inf_id,
                                         jsonCatalog[inf_id]["name"].GetString(),
                                         jsonCatalog[inf_id]["thumbnail"].GetString());
        
        const InfluencerData& inf_data = jsonCatalog[inf_id];
        
        for (rapidjson::Value::ConstMemberIterator itr = inf_data["videos"].MemberBegin();
             itr != inf_data["videos"].MemberEnd();
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

        influencers.push_front(inf);
    }
    
    int i = 0;
    for (auto& inf: influencers) {
        auto item = MenuItemFont::create(inf->getId(),
                                         CC_CALLBACK_1(PlayClipsSample::onInfluencerSelected, this));
        
        item->setName(inf->getId());
        item->setFontNameObj("Arial");
        item->setFontSizeObj(14);
        item->setAnchorPoint(Vec2(0, 0));
        item->setPosition(Vec2(origin.x+10, origin.y + 30*(i++)+10));
        influencerNames.pushBack(item);
    }
    
    auto menu = Menu::createWithArray(influencerNames);
    menu->setAnchorPoint(Vec2(0, 1));
    menu->setPosition(Vec2(origin.x,
                           sprite->getPosition().y - 50 - sprite->getContentSize().height / sprite->getScale()
                           )
                      );
    this->addChild(menu, 1);
}

template<typename R, typename C, typename P>
void mapWithIndex(R& r, const C& c, P pred) {
    int idx = 0;
    for (auto& elem : c) {
        r.pushBack(pred(elem, idx));
        idx++;
    }
}

template<typename C, typename P>
void forEachWithIndex(const C& c, P pred) {
    int idx = 0;
    for (auto& elem : c) {
        pred(elem, idx);
    }
}

template<typename C, typename P>
void forEach(const C& c, P pred) {
    for (auto& elem : c) {
        pred(elem);
    }
}

template <typename C, typename P>
Influencer* findFirst(const C& c, P pred) {
    for (auto& inf: c) {
        if (pred(*inf)) {
            return inf;
        }
    }
    return nullptr;
}

void PlayClipsSample::onInfluencerSelected(Ref* pSender) {
    MenuItemFont* item = (MenuItemFont *)pSender;
    Vector<MenuItem*> tagsMenu;
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    const Influencer* influencer = findFirst(influencers,
                                             [&item](const Influencer& inf) {
                                                 return inf.getId() == item->getName();
                                             }
                                             );
    cocos2d::log("%s", influencer->getId().c_str());
    
    std::set<std::string> tags;
    
    forEach(influencer->getVideos(), [&tags](Video* video) {
        forEach(video->getTags(), [&tags](std::string tag){
            tags.insert(tag);
        });
    });
    
    cocos2d::log("Tags set size is %lu", tags.size());
    
    mapWithIndex(tagsMenu, tags, [&origin, &influencer, this](std::string tag, int idx) {
        auto item = MenuItemFont::create(tag,
                                         CC_CALLBACK_1(PlayClipsSample::playVideo,
                                                       this,
                                                       influencer,
                                                       tag));
        item->setName(tag);
        item->setFontNameObj("Arial");
        item->setFontSizeObj(14);
        item->setAnchorPoint(Vec2(0, 1));
        item->setPosition(Vec2(origin.x+10, origin.y - 30*idx-10));
        return item;
    });

    auto menu = Menu::createWithArray(tagsMenu);
    menu->setAnchorPoint(Vec2(0, 1));
    menu->setPosition(Vec2(origin.x + 100, sprite->getPosition().y - 50 - sprite->getContentSize().height));
    
    this->addChild(menu, 10);
    
}

void PlayClipsSample::playVideo(Ref* pSender, const Influencer* inf, std::string tag) {
    Video* video = inf->getVideoByTag(tag);
    std::string str = video->getLocation();

    // TODO: user to be able to select the quality, either high, medium or low
    str.replace(str.find("{quality}"), 9, "high");
    
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
    
    // TODO: play the requested video instead of a hardcoded one
    videoPlayer->setFileName("welcome.mp4");
    videoPlayer->setVisible(true);
    videoPlayer->setFullScreenEnabled(false);
    this->addChild(videoPlayer, 10);
    videoPlayer->play();
#else
    std::string unsupportedPlatform = "I was about to reproduce "+video->getId()+", but your platform does not support Video player";
    cocos2d::log("%s", unsupportedPlatform.c_str());
    MessageBox(unsupportedPlatform.c_str(), "About");
#endif
    
}

void PlayClipsSample::loadCatalog(Ref* pSender)
{
    menuStart->setVisible(false);
    
// TODO: check connectivity at start to avoid this requirement
# if CONNECT_TO_CDN
    const char* CATALOG_URL = "http://cdn-stg.svplayclips.com/metadata.json";
    
    network::HttpRequest* request = new (std :: nothrow)network::HttpRequest();
    request->setUrl(CATALOG_URL);
    request->setRequestType(network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(PlayClipsSample::onHttpRequestCatalog, this));
    cocos2d::network::HttpClient::getInstance()->sendImmediate(request);
    cocos2d::log("Obtaining catalog from %s", CATALOG_URL);
    request->release();
    return;
#endif
    cocos2d::log("Loading data from disk");
    // Load catalog locally (out of coverage)
    const char * data = FileUtils::getInstance()->getStringFromFile("catalog.json").c_str();
    loadJsonCatalog(data);
}
