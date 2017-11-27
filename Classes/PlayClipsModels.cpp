#include <iostream>
#include "PlayClipsModels.h"
#include <algorithm>

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
