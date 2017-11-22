#pragma once

#include <iostream>
#include <set>
#include <list>

class Video {

public:
    Video(std::string id, std::string location, int weight):
    video_id {id}, location {location}, weight {weight} {};

    std::string getId() const { return video_id; };

    std::string getLocation() const { return location; };

    std::set<std::string> getTags() { return tags; };

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

    std::list<Video*> getVideos() const { return videos; };

    void addVideo(Video* video) { videos.push_back(video); }

    std::list<Video*> getVideos() { return videos; }

    ~Influencer() {
        for (auto& video: videos) {
            delete video;
        }
    }

private:
    std::string influencer_id;
    std::string name;
    std::string thumbnail;
    std::list<Video*> videos;
};
