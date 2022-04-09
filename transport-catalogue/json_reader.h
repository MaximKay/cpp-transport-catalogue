#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include <sstream>

#include "geo.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"

namespace json {
	using namespace std::string_literals;

	class JsonReader {
	public:
		JsonReader() = default;

		void LoadData(std::istream&, render::MapRenderer& renderer);

		void Print(std::ostream& out, const std::vector<objects::RequestAnswer>&);

		const std::map<std::string, geo::Coordinates>& GetParsedStops();

		const std::vector <objects::Request>& GetParsedRequests();

		const std::map<std::string, std::map<std::string, int>>& GetRoutesLengths();

		const std::map<std::string, std::pair<std::vector<std::string>, bool>>& GetParsedBuses();

	private:
		std::map<std::string, geo::Coordinates> parsed_stops_{};
		std::map<std::string, std::pair<std::vector<std::string>, bool>> parsed_buses_routes_{};
		std::map<std::string, std::map<std::string, int>> routes_lengths_{};
		std::vector <objects::Request> parsed_requests{};

		void ProcessStopsAndBuses(std::vector<Node>);

		void ProcessRenderSettings(render::MapRenderer& renderer, std::map<std::string, Node>);

		void ProcessRequests(std::vector<Node>);

		void ParseStops(std::vector<Dict>);

		void ParseBuses(std::vector<Dict>);
	};

}//end of namespace json