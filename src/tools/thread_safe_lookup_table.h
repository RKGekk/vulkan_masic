#pragma once

#include <list>
#include <memory>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename KeyType, typename ValueType>
class ThreadSafeLookupTable {
public:
    using HashFn = std::hash<KeyType>;

    ThreadSafeLookupTable(const unsigned num_buckets_ = 16, const HashFn& hasher_ = HashFn{}) : m_buckets(num_buckets_), m_hasher_fn(hasher_) {
        for(unsigned i = 0; i < num_buckets_; ++i) {
            m_buckets[i].reset(new bucket_type);
        }
    }

    ThreadSafeLookupTable(const ThreadSafeLookupTable& other) = delete;
    ThreadSafeLookupTable& operator=(const ThreadSafeLookupTable& other) = delete;
    ThreadSafeLookupTable(ThreadSafeLookupTable&& other) = delete;
    ThreadSafeLookupTable& operator=(ThreadSafeLookupTable&& other) = delete;

    ValueType ValueFor(const KeyType& key, const ValueType& default_value = ValueType()) const {
        return get_bucket(key).value_for(key, default_value);
    }

    void AddOrUpdateMapping(const KeyType& key, const ValueType& value) {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void RemoveMapping(const KeyType& key) {
        get_bucket(key).remove_mapping(key);
    }

    std::map<KeyType, ValueType> getMap() const {
        std::vector<std::unique_lock<std::shared_mutex>> locks;
        size_t sz = m_buckets.size();
        for(unsigned i = 0; i < sz; ++i) {
            locks.push_back(std::unique_lock<std::shared_mutex>(m_buckets[i]->get_mutex()));
        }
        std::map<KeyType, ValueType> res;
        for(unsigned i = 0; i < sz; ++i) {
            const bucket_type::bucke_data& bucket_data = m_buckets[i]->get_data();
            for(auto it = bucket_data.cbegin(); it != bucket_data.cend(); ++it) {
                res.insert(*it);
            }
        }
        return res;
    }

    std::unordered_map<KeyType, ValueType> getUnorderedMap() const {
        std::vector<std::unique_lock<std::shared_mutex>> locks;
        size_t sz = m_buckets.size();
        for(unsigned i = 0; i < sz; ++i) {
            locks.push_back(std::unique_lock<std::shared_mutex>(m_buckets[i]->get_mutex()));
        }
        std::unordered_map<KeyType, ValueType> res;
        res.reserve(sz);
        for(unsigned i = 0; i < sz; ++i) {
            const bucket_type::bucke_data& bucket_data = m_buckets[i]->get_data();
            for(auto it = bucket_data.cbegin(); it != bucket_data.cend(); ++it) {
                res.insert(*it);
            }
        }
        return res;
    }

private:
    class bucket_type {
    public:
        using bucket_value = std::pair<KeyType, ValueType>;
        using bucke_data = std::list<bucket_value>;
        using bucket_it = bucke_data::iterator;
        using bucket_const_it = bucke_data::const_iterator;

        ValueType value_for(const KeyType& key, const ValueType& default_value) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            bucket_const_it found_entry = find_entry_for(key);
            return (found_entry == m_data.cend()) ? default_value : found_entry->second;
        }

        void add_or_update_mapping(const KeyType& key, const ValueType& value) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            bucket_it found_entry = find_entry_for(key);
            if(found_entry == m_data.end()) {
                m_data.push_back(bucket_value(key, value));
            }
            else {
                found_entry->second = value;
            }
        }

        void remove_mapping(const KeyType& key) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            bucket_const_it found_entry = find_entry_for(key);
            if(found_entry == m_data.cend()) {
                m_data.erase(found_entry);
            }
        }

        std::shared_mutex& get_mutex() const {
            return m_mutex;
        }

        const bucke_data& get_data() const {
            return m_data;
        }

    private:
        bucke_data m_data;
        mutable std::shared_mutex m_mutex;

        bucket_const_it find_entry_for(const KeyType& key) const {
            return std::find_if(
                m_data.cbegin(),
                m_data.cend(),
                [&](const bucket_value& item) {
                    return item.first == key;
                }
            );
        }

        bucket_it find_entry_for(const KeyType& key) {
            return std::find_if(
                m_data.begin(),
                m_data.end(),
                [&](const bucket_value& item) {
                    return item.first == key;
                }
            );
        }
    };

    std::vector<std::unique_ptr<bucket_type>> m_buckets;
    const HashFn m_hasher_fn;

    const bucket_type& get_bucket(const KeyType& key) const {
        const std::size_t bucket_index = m_hasher_fn(key) % m_buckets.size();
        return *m_buckets[bucket_index];
    }

    bucket_type& get_bucket(const KeyType& key) {
        const std::size_t bucket_index = m_hasher_fn(key) % m_buckets.size();
        return *m_buckets[bucket_index];
    }
};