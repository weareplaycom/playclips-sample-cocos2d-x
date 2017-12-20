#pragma once

#include <list>

#include "cocos2d.h"
#include "network/HttpClient.h"
#include "json/rapidjson.h"
#include "json/document-wrapper.h"

#include "Adjust/Adjust2dx.h"

#include "PlayClipsModels.h"

using namespace cocos2d::network;


class PlayClipsSample : public cocos2d::Scene
{
public:
    virtual bool init() override;

    static Scene* createScene();

    void menuCloseCallback(cocos2d::Ref* pSender);

    // Download the influencer asset catalog using Asset Manager Ex
    void downloadZipCatalog(std::string influencer);

    // static method to handle the deferred deeplink information obtained
    // from Adjust
    static bool deferredDeeplinkCallbackMethod(std::string deeplink);

    static void attributionCallbackMethod(AdjustAttribution2dx attribution);

    static void eventSuccessCallbackMethod(AdjustEventSuccess2dx eventSuccess);

    static void eventFailureCallbackMethod(AdjustEventFailure2dx eventFailure);

    void playVideo(cocos2d::Ref* pSender, std::string tag);
    
    // implement the "static create()" method manually
    CREATE_FUNC(PlayClipsSample);
    
    void update(float) override;
    
private:
    void influencerSelected();
    void loadInfluencerMetadata(cocos2d::Ref* pSender);

    cocos2d::Menu* menuStart;
    cocos2d::Sprite* sprite;
    rapidjson::Document jsonCatalog;
    Influencer* influencer;    
};
