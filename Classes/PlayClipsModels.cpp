#include <iostream>
#include "PlayClipsModels.h"


Video* Influencer::getVideoByTag (std::string tag) const {

    for (auto& video: this->getVideos()) {
        for (auto& videoTag: video->getTags()) {
            if (tag == videoTag) {
                cocos2d::log("Use this video: %s", video->getId().c_str());
                return video;

            }
        }
    }
    // tag not found!
    throw new std::exception();
}
