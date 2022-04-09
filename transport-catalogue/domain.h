#pragma once
#include <string>
#include <cmath>
#include <string_view>
#include "geo.h"

namespace objects{
	struct Bus {
		Bus(const std::string& bus_name, bool is_roundtrip) : bus_name_(bus_name), is_roundtrip_(is_roundtrip) {
		}

		bool operator==(const std::string& other_bus) {
			return other_bus == bus_name_;
		}

		std::string bus_name_;
		bool is_roundtrip_;
	};

	struct BusPtrComp
	{
		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->bus_name_ < rhs->bus_name_;
		}
	};

	struct Stop {
		Stop(const std::string& name, const geo::Coordinates& point) :
			stop_(name), point_(point) {
		}

		bool operator==(const std::string& other_stop) {
			return other_stop == stop_;
		}

		std::string stop_;
		geo::Coordinates point_;
	};

	struct StopPtrComp
	{
		bool operator()(const Stop* lhs, const Stop* rhs) const {
			return lhs->stop_ < rhs->stop_;
		}
	};

	struct StopsPtrsPair {
		StopsPtrsPair(const Stop* ptr1, const Stop* ptr2) : first(ptr1), second(ptr2) {
		}

		bool operator==(const StopsPtrsPair& other_stop_pair) const {
			return first->stop_ == other_stop_pair.first->stop_ &&
				second->stop_ == other_stop_pair.second->stop_;
		}

		const Stop* first{};
		const Stop* second{};
	};

	struct StopsPtrsPairHasher {
		size_t operator()(const StopsPtrsPair& stops_pair) const {
			int index{}, i{ 1 };
			for (const char c : (stops_pair.first->stop_ + stops_pair.second->stop_)) {
				index += (c - '0') * static_cast<int>(std::pow(37, i));
				++i;
			};
			return static_cast<size_t>(index);
		}
	};

	struct RouteData {
		size_t unique_stops{};
		size_t stops_count{};
		size_t length{};
		double curvature{};
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