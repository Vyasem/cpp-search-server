#pragma once
#include <future>
#include <map>
template <typename Key, typename Value>
class ConcurrentMap {
private:
	struct ItemMap {
		std::mutex mutex;
		std::map<Key, Value> data;
	};
	std::vector<ItemMap> subMaps;
public:
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

	struct Access {
		std::lock_guard<std::mutex> guard;
		Value& tdIdf;
		Access(const Key& key, ItemMap& bucket) : guard(bucket.mutex), tdIdf(bucket.data[key]) {}
	};

	explicit ConcurrentMap(size_t bucket_count) :subMaps(bucket_count) {};

	Access operator[](const Key& key) {
		auto& bucket = subMaps[static_cast<uint64_t>(key) % subMaps.size()];
		return { key, bucket };
	};

	std::vector<ItemMap>& BuildOrdinaryMap() {
		return subMaps;
	};
};