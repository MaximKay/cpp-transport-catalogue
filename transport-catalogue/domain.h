#pragma once
#include <string>
#include <cmath>
#include <string_view>
#include <vector>
#include <set>
#include <variant>

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
		Bus(const std::string& bus, bool is_round);

		bool operator==(const std::string& other_bus);

		std::string name;
		bool is_roundtrip;
		std::vector<const Stop*> stops{};
		RouteData route_data;
	};

	struct BusPtrComp
	{
		bool operator()(const Bus* lhs, const Bus* rhs) const;
	};

	struct Stop {
		Stop(const std::string& stop, const geo::Coordinates& init_coordinates);

		bool operator==(const std::string& other_stop);

		std::string name;
		geo::Coordinates coordinates;
		std::set<const Bus*, BusPtrComp> buses{};
	};

	struct StopPtrComp
	{
		bool operator()(const Stop* lhs, const Stop* rhs) const;
	};

	struct StopsPtrsPair {
		StopsPtrsPair(const Stop* ptr1, const Stop* ptr2);

		bool operator==(const StopsPtrsPair& other_stops) const;

		const Stop* first{};
		const Stop* second{};
	};

	struct StopsPtrsPairHasher {
		size_t operator()(const StopsPtrsPair& stops_pair) const;
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