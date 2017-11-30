#pragma once

#include <vector>
#include <set>

#include "network/Uri.h"

class Video {

public:
    Video(std::string id, std::string location, int weight):
    video_id {id}, location {location}, weight {weight} {};

    std::string getId() const { return video_id; };

    std::string getLocation() const { return location; };

    std::set<std::string> getTags() const { return tags; };

    void addTag(std::string tag) { tags.insert(tag); };

    bool hasTag(std::string tag) const { return !tags.find(tag)->empty(); };

private:
    std::string video_id;
    std::string location;
    std::set<std::string> tags;
    int weight;
};

class Influencer {

public:

    Influencer(std::string id, std::string name, std::string thumbnail):
    influencer_id {id}, name {name}, thumbnail {thumbnail} {};

    std::string getId() const { return influencer_id; };

    std::string getName() const { return name; };

    std::string getThumbnail() const { return thumbnail; };

    void addVideo(Video* video) { videos.push_back(video); };

    Video* getVideoByTag (std::string tag) const;

    std::vector<Video*> getVideos() const { return videos; }

    ~Influencer() {
        for (auto& video: videos) {
            delete video;
        }
    }

private:
    std::string influencer_id;
    std::string name;
    std::string thumbnail;
    std::vector<Video*> videos;
};

class DeferredDeeplink {
public:
    DeferredDeeplink(std::string deeplink);
    std::string getInfluencer();
    bool isValid();

private:
    std::string deeplink;
    cocos2d::network::Uri deeplinkUri;
};
