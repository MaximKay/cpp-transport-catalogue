#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <algorithm>
#include <cmath>
#include <set>
#include <deque>
#include <variant>

#include "geo.h"
#include "domain.h"

using namespace std::string_literals;
using namespace objects;

namespace transport {
	class Catalogue {
	public:
		Catalogue() = default;

		void ParseRoutesLengths(const std::map<std::string, std::map<std::string, int>>&);

		void AddStop(const std::string&, const geo::Coordinates&);

		const Bus* AddBus(const std::string&, const bool);

		void ExpandBusAndStopInfo(const Bus*, const Stop*);

		void ExpandBusAndStopInfo(const Stop*, const Bus*);

		const Stop* FindStop(const std::string&);

		const Bus* FindBus(const std::string&);

		const std::set<const Bus*, BusPtrComp>& GetBusesForStop(const Stop*);

		void SetStopsDistance(const Stop*, const Stop*, const size_t);

		size_t GetStopsDistance(const Stop*, const Stop*);

		void CalculateRoutesData();

		const RequestAnswer ConstructAnswerForRequest(const Request&);

		const std::vector<const Bus*> RoutesForMap();

	private:
		std::unordered_map<const StopsPtrsPair, size_t, StopsPtrsPairHasher> routes_lengths_{};
		std::deque<Stop> stops_{};
		std::deque<Bus> buses_{};
	};
}