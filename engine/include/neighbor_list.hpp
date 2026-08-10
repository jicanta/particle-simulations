#pragma once

#include <cstddef>
#include <vector>

class NeighborList {
public:
    explicit NeighborList(std::size_t particleCount);

    void addPair(int first, int second);
    void sortEachEntry();

    const std::vector<int>& of(std::size_t particleId) const;
    std::size_t size() const;
    std::size_t pairCount() const;

    bool operator==(const NeighborList& other) const;

private:
    std::vector<std::vector<int>> neighbors_;
};
