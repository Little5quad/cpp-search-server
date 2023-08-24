#pragma once

#include <mutex>
#include <map>



template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket{
        std::mutex m_buck;
        std::map<Key, Value> map;
    };
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket) : guard(bucket.m_buck), ref_to_value(bucket.map[key]){
        }
        void operator+=(const Value& val) {
            ref_to_value += val;
        }
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count){
    }

    Access operator[](const Key& key){
        auto& bucket = buckets_[static_cast<uint64_t>(key) % buckets_.size()];
        return {key, bucket};
    }
    
    void Erase(Key key){
        size_t bucket_index = static_cast<size_t>(key) % buckets_.size();
        buckets_[bucket_index].map.erase(key);
    }
    
    std::map<Key, Value> BuildOrdinaryMap(){
        std::map<Key, Value> result;
        for (auto& [mut, map] : buckets_){
            std::lock_guard g(mut);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private:
    std::vector<Bucket> buckets_;
};
