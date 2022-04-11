#include "transport_catalogue.h"

using namespace objects;

namespace transport {
	void Catalogue::ParseRoutesLengths(const std::map<std::string, std::map<std::string, int>>& routes) {
		for (const auto& [first_stop, stops_with_length] : routes) {
			const Stop* stop_a = FindStop(first_stop);
			for (const auto& [second_stop, length] : stops_with_length) {
				const Stop* stop_b = FindStop(second_stop);
				const StopsPtrsPair stops_pair = { stop_a, stop_b };
				routes_lengths_[stops_pair] = length;
			};
		};
	}

	void Catalogue::AddStop(const std::string& name, const geo::Coordinates& point) {
		stops_.emplace_back(name, point);
	}

	const Bus* Catalogue::AddBus(const std::string& name, const bool is_roundtrip) {
		return &(buses_.emplace_back(name, is_roundtrip));
	}

	void Catalogue::ExpandBusAndStopInfo(const Bus* bus_ptr, const Stop* stop_ptr) {
		const_cast<Bus*>(bus_ptr)->stops.push_back(stop_ptr);
		const_cast<Stop*>(stop_ptr)->buses.insert(bus_ptr);
	}

	void Catalogue::ExpandBusAndStopInfo(const Stop* stop_ptr, const Bus* bus_ptr) {
		const_cast<Stop*>(stop_ptr)->buses.insert(bus_ptr);
		const_cast<Bus*>(bus_ptr)->stops.push_back(stop_ptr);
	}

	const Stop* Catalogue::FindStop(const std::string& stop) {
		auto search_res = find(stops_.begin(), stops_.end(), stop);
		return search_res == stops_.end() ? nullptr : &(*search_res);
	}

	const Bus* Catalogue::FindBus(const std::string& bus) {
		auto search_res = find(buses_.begin(), buses_.end(), bus);
		return search_res == buses_.end() ? nullptr : &(*search_res);
	}

	const std::set<const Bus*, BusPtrComp>& Catalogue::GetBusesForStop(const Stop* stop_ptr) {
		return stop_ptr->buses;
	}

	void Catalogue::SetStopsDistance(const Stop* a,
		const Stop* b, const size_t length) {
		routes_lengths_[StopsPtrsPair(a, b)] = length;
	}

	size_t Catalogue::GetStopsDistance(const Stop* a, const Stop* b) {
		return routes_lengths_.at(StopsPtrsPair(a, b));
	}

	void Catalogue::CalculateRoutesData() {
		for (auto& bus : buses_) {
			RouteData data{};
			//getting count of unique stops in separate namespace to save memory
			{
				data.unique_stops = std::set(bus.stops.begin(), bus.stops.end()).size();
			}

			//getting stop count for bus
			data.stops_count = bus.stops.size();

			//getting sum of the lengths between stops pointers and curvature
			const auto& stops = bus.stops;
			double curvatures{};
			for (size_t i = 0; i < stops.size() - 1; ++i) {
				//if length between stops as in bus route doesnt exist, getting length of reversed stops
				if (routes_lengths_.count({ stops.at(i), stops.at(i + 1) })) {
					data.length += routes_lengths_.at({ stops.at(i), stops.at(i + 1) });
				}
				else if (routes_lengths_.count({ stops.at(i + 1), stops.at(i) })) {
					data.length += routes_lengths_.at({ stops.at(i + 1), stops.at(i) });
				};
				//summing computed curvatures
				curvatures += geo::ComputeDistance(stops.at(i)->coordinates, stops.at(i + 1)->coordinates);
			};
			//curvature of all route. in case if length is zero, curvature will be zero too
			data.curvature = (data.length == 0) ? 0 : data.length / curvatures;
			//writing all data to catalogue
			bus.route_data = data;
		};
	}

	const RequestAnswer Catalogue::ConstructAnswerForRequest(const Request& request) {

		RequestAnswer answer;
		answer.id = request.id;

		if (request.type == "Stop"s) {
			//getting stop pointer so we can find all buses for it
			const Stop* stop_ptr = FindStop(request.name);
			if (stop_ptr == nullptr) {
				//if stop doesnt exist, error as bool 'false' will be instead of buses 
				answer.data = false;
			}
			else {
				std::vector<std::string> buses;
				//in case if stop doesnt have buses, adding empty array
				if (!(stop_ptr->buses.empty())) {
					//converting pointers to strings, filling vector of buses
					for (const auto& bus_ptr : stop_ptr->buses) {
						buses.push_back(bus_ptr->name);
					};
				};
				answer.data = buses;
			};
		}
		else if (request.type == "Bus"s) {
			//getting pointer to the bus
			const Bus* bus_ptr = FindBus(request.name);
			if (bus_ptr == nullptr) {
				//if bus doesnt exist, then add error as bool 'false'
				answer.data = false;
			}
			else {
				//if bus exists getting route data
				answer.data = bus_ptr->route_data;
			};
		};

		return answer;
	}

	const std::vector<const Bus*> Catalogue::RoutesForMap() {
		std::set<const Bus*, BusPtrComp> routes;
		for (const auto& bus : buses_) {
			routes.insert(&bus);
		};
		return std::vector<const Bus*>(routes.begin(), routes.end());
	}
}