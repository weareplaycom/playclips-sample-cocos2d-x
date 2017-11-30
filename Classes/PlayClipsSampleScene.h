#pragma once

#include "cocos2d.h"
#include <list>
#include "network/HttpClient.h"
#include "json/rapidjson.h"
#include "json/document-wrapper.h"

#include "PlayClipsModels.h"

using namespace cocos2d::network;


class PlayClipsSample : public cocos2d::Scene
{
public:
    virtual bool init() override;

    static Scene* createScene();

    void downloadZipCatalog(std::string influencer);

    static bool deferredDeeplinkCallbackMethod(std::string deeplink);

    void menuCloseCallback(cocos2d::Ref* pSender);
    
    void influencerSelected();

    void loadCatalog(cocos2d::Ref* pSender);
    void playVideo(cocos2d::Ref* pSender, std::string tag);
    
    void loadJsonCatalog(const char* json, std::string influencer_id);
    
    // implement the "static create()" method manually
    CREATE_FUNC(PlayClipsSample);
    
    void update(float) override;
    
private:
    cocos2d::Menu* menuStart;
    cocos2d::Sprite* sprite;
    rapidjson::Document jsonCatalog;
    Influencer* influencer;    
};
