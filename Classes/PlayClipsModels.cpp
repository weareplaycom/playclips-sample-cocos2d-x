#include <iostream>
#include "PlayClipsModels.h"
#include <algorithm>

const std::string INFLUENCER_QUERY_PARAM = "influencer";

/*
 Return the first video that is tagged with the tag received as parameter
 */
Video* Influencer::getVideoByTag (std::string tag) const {
    return *std::find_if(this->getVideos().begin(),
                         this->getVideos().end(),
                         [&tag](Video* video) {
                             // Select this video if it holds
                             // the tag <<tag>>
                             std::set<std::string> tags = video->getTags();
                             return std::find(tags.begin(),
                                              tags.end(),
                                              tag)
                                    != tags.end();
                         });
}

DeferredDeeplink::DeferredDeeplink(std::string deeplink) {
    // Obtain influencer and load asset catalog
    this->deeplinkUri = cocos2d::network::Uri::parse(deeplink);
}


bool DeferredDeeplink::isValid() {
    auto influencerQueryParam = std::find_if(this->deeplinkUri.getQueryParams().begin(),
                                             this->deeplinkUri.getQueryParams().end(),
                                             [](std::pair<std::string, std::string> element) {
                                                 return element.first == INFLUENCER_QUERY_PARAM;
                                             });
    return influencerQueryParam != this->deeplinkUri.getQueryParams().end();
}

std::string DeferredDeeplink::getInfluencer() {
    auto influencerQueryParam = std::find_if(this->deeplinkUri.getQueryParams().begin(),
                                             this->deeplinkUri.getQueryParams().end(),
                                             [](std::pair<std::string, std::string> element) {
                                                 return element.first == INFLUENCER_QUERY_PARAM;
                                             });
    return influencerQueryParam->second.c_str();
}
