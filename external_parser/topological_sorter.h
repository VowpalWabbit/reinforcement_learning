#pragma once

#include <list>
#include <queue>
#include <unordered_map>

template<
    typename value_t,
    typename key_pri_t=std::string,
    typename key_sec_t=int>
class topological_sorter {
public:
    using iterator = std::vector<primary_key_t*>::const_iterator;

public:
    topological_sorter() = default;

    topological_sorter(const topological_sorter& other) = delete;
    topological_sorter& operator=(const topological_sorter&) = default;

    topological_sorter(topological_sorter&&) = default;
    topological_sorter& operator=(topological_sorter&&) = default;

    void add(const primary_key_t* primary, const secondary_key_t& secondary, const primary_key_t& previous) {
        _vertices.push_back(primary);
    }

    iterator begin() const {
        return _vertices.begin();
    }

    iterator end() const {
        return _vertices.end();
    }    

private:
    std::vector<primary_key_t*> _vertices;
    
};