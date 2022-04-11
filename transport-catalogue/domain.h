#pragma once
#include <string>
#include <cmath>
#include <string_view>
#include <vector>
#include <set>

#include "geo.h"

namespace objects {
	struct RouteData {
		size_t unique_stops{};
		size_t stops_count{};
		size_t length{};
		double curvature{};
	};

	struct Stop;

	struct Bus {
		Bus(const std::string& bus, bool is_round) : name(bus), is_roundtrip(is_round) {
		}

		bool operator==(const std::string& other_bus) {
			return other_bus == name;
		}

		std::string name;
		bool is_roundtrip;
		std::vector<const Stop*> stops{};
		RouteData route_data;
	};

	struct BusPtrComp
	{
		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	struct Stop {
		Stop(const std::string& stop, const geo::Coordinates& init_coordinates) :
			name(stop), coordinates(init_coordinates) {
		}

		bool operator==(const std::string& other_stop) {
			return other_stop == name;
		}

		std::string name;
		geo::Coordinates coordinates;
		std::set<const Bus*, BusPtrComp> buses{};
	};

	struct StopPtrComp
	{
		bool operator()(const Stop* lhs, const Stop* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	struct StopsPtrsPair {
		StopsPtrsPair(const Stop* ptr1, const Stop* ptr2) : first(ptr1), second(ptr2) {
		}

		bool operator==(const StopsPtrsPair& other_stops) const {
			return (other_stops.first == first) && (other_stops.second == second);
		}

		const Stop* first{};
		const Stop* second{};
	};

	struct StopsPtrsPairHasher {
		size_t operator()(const StopsPtrsPair& stops_pair) const {
			int index{}, i{ 1 };
			for (const char c : (stops_pair.first->name + stops_pair.second->name)) {
				index += (c - '0') * static_cast<int>(std::pow(37, i));
				++i;
			};
			return static_cast<size_t>(index);
		}
	};

	struct Request {
		int id{};
		std::string type{};
		std::string name{};
	};

	//object can be a stop, a bus, error message or rendering options: 
	//for stop answer will be a vector of buses names, 
	//for bus will be RouteData struct
	//for error will be bool false
	//for rendering options will be string_view
	struct RequestAnswer {
		int id{};
		std::variant<bool, std::string_view, std::vector<std::string>, RouteData> data;
	};
}//end objects namespace