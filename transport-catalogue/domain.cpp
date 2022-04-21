#include "domain.h"

namespace objects {

	Bus::Bus(const std::string& bus, bool is_round) : name(bus), is_roundtrip(is_round) {
	}

	bool Bus::operator==(const std::string& other_bus) {
		return other_bus == name;
	}

	bool BusPtrComp::operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}

	Stop::Stop(const std::string& stop, const geo::Coordinates& init_coordinates) :
		name(stop), coordinates(init_coordinates) {
	}

	bool Stop::operator==(const std::string& other_stop) {
		return other_stop == name;
	}

	bool StopPtrComp::operator()(const Stop* lhs, const Stop* rhs) const {
		return lhs->name < rhs->name;
	}

	StopsPtrsPair::StopsPtrsPair(const Stop* ptr1, const Stop* ptr2) : first(ptr1), second(ptr2) {
	}

	bool StopsPtrsPair::operator==(const StopsPtrsPair& other_stops) const {
		return (other_stops.first == first) && (other_stops.second == second);
	}

	size_t StopsPtrsPairHasher::operator()(const StopsPtrsPair& stops_pair) const {
		int index{}, i{ 1 };
		for (const char c : (stops_pair.first->name + stops_pair.second->name)) {
			index += (c - '0') * static_cast<int>(std::pow(37, i));
			++i;
		};
		return static_cast<size_t>(index);
	}

}