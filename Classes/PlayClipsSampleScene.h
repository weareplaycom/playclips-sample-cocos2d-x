#pragma once

#include "cocos2d.h"
#include "network/HttpClient.h"
#include "json/rapidjson.h"
#include "json/document-wrapper.h"

using namespace cocos2d::network;


class PlayClipsSample : public cocos2d::Scene
{
public:
    virtual bool init() override;
    
    static cocos2d::Scene* createScene();

    virtual void menuCloseCallback(cocos2d::Ref* pSender);
    
    virtual void onInfluencerSelected(cocos2d::Ref *pSender);
    
    virtual void loadCatalog(cocos2d::Ref* pSender);
    virtual void playVideo(cocos2d::Ref* pSender, const char* location);
    
    virtual void onHttpRequestCatalog(HttpClient *sender, HttpResponse *response);
    virtual void loadJsonCatalog(const char* json);
    
    // implement the "static create()" method manually
    CREATE_FUNC(PlayClipsSample);
    
    void update(float) override;
    
private:
    cocos2d::Menu* menuStart;
    cocos2d::Sprite* sprite;
    rapidjson::Document jsonCatalog;

};
