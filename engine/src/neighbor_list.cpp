#include "neighbor_list.hpp"

#include <algorithm>

NeighborList::NeighborList(std::size_t particleCount) : neighbors_(particleCount) {}

void NeighborList::addPair(int first, int second) {
    neighbors_[static_cast<std::size_t>(first)].push_back(second);
    neighbors_[static_cast<std::size_t>(second)].push_back(first);
}

void NeighborList::sortEachEntry() {
    for (std::vector<int>& entry : neighbors_) {
        std::sort(entry.begin(), entry.end());
    }
}

const std::vector<int>& NeighborList::of(std::size_t particleId) const {
    return neighbors_[particleId];
}

std::size_t NeighborList::size() const {
    return neighbors_.size();
}

std::size_t NeighborList::pairCount() const {
    std::size_t total = 0;
    for (const std::vector<int>& entry : neighbors_) {
        total += entry.size();
    }
    return total / 2;
}

bool NeighborList::operator==(const NeighborList& other) const {
    return neighbors_ == other.neighbors_;
}
