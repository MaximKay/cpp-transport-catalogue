#include "json_reader.h"

namespace json {

	using namespace std::string_literals;

	void JsonReader::LoadData(std::istream& input, render::MapRenderer& renderer) {
		Document doc = Load(input);

		const auto& all_requests = doc.GetRoot().AsMap();

		//process base requests
		ProcessStopsAndBuses(all_requests.at("base_requests"s).AsArray());
		//process render settings
		ProcessRenderSettings(renderer, all_requests.at("render_settings"s).AsMap());
		//process stat requests
		ProcessRequests(all_requests.at("stat_requests"s).AsArray());
	}

	void JsonReader::Print(std::ostream& out, const std::vector<objects::RequestAnswer>& data) {
		out << '[' << std::endl;

		using stop_data = std::vector<std::string>;
		using bus_data = objects::RouteData;
		using svg_map = std::string_view;
		using error = bool;

		bool is_first = true; //bool to identify first object
		for (const auto& answer : data) {
			if (!is_first) { out << ',' << std::endl; }; //if object is not first - add comma and new line
			is_first = false;

			out << '{' << std::endl;
			out << "\"request_id\": "s << answer.id << ',' << std::endl;

			//if data is vector - object is stop
			if (std::holds_alternative<stop_data>(answer.data)) {
				PrintStop(out, std::get<stop_data>(answer.data));
			}
			//if data is RouteData - object is bus
			else if (std::holds_alternative<bus_data>(answer.data)) {
				PrintBus(out, std::get<bus_data>(answer.data));
			}
			//case if data is map_renderer(string_view)
			else if (std::holds_alternative<svg_map>(answer.data)) {
				PrintSvgMap(out, std::get<svg_map>(answer.data));
			}
			//id data is error (bool) - object is error
			else if (std::holds_alternative<error>(answer.data)) {
				out << "\"error_message\": \"not found\""s << std::endl;
			};
			out << '}';
		};
		out << std::endl << ']' << std::endl;
	}

	void JsonReader::PrintStop(std::ostream& out, const std::vector<std::string>& buses) {
		out << "\"buses\": "s << '[' << std::endl;
		bool first = true;
		for (const auto& bus : buses) {
			if (first) {
				out << '\"' << bus << '\"';
				first = false;
			}
			else {
				out << ", "s << '\"' << bus << '\"';
			};
		};
		out << std::endl << ']' << std::endl;
	}

	void JsonReader::PrintBus(std::ostream& out, const objects::RouteData& data) {
		out << "\"curvature\": "s << data.curvature << ',' << std::endl;
		out << "\"route_length\": "s << data.length << ',' << std::endl;
		out << "\"stop_count\": "s << data.stops_count << ',' << std::endl;
		out << "\"unique_stop_count\": "s << data.unique_stops << std::endl;
	}

	void JsonReader::PrintSvgMap(std::ostream& out, const std::string_view& svg_map) {
		out << "\"map\": \""s;
		//processing string_view char by char to catch escape sequences
		for (const char c : svg_map) {
			if (c == '\"' || c == '\\') {
				out << '\\';
			}
			else if (c == '\n') {
				out << "\\n"s;
				continue;
			};
			out << c;
		};
		out << '\"' << std::endl;
	}

	const std::map<std::string, geo::Coordinates>& JsonReader::GetParsedStops() {
		return parsed_stops_;
	}

	const std::vector <objects::Request>& JsonReader::GetParsedRequests() {
		return parsed_requests;
	}

	const std::map<std::string, std::map<std::string, int>>& JsonReader::GetRoutesLengths() {
		return routes_lengths_;
	}

	const std::map<std::string, std::pair<std::vector<std::string>, bool>>& JsonReader::GetParsedBuses() {
		return parsed_buses_routes_;
	}

	void JsonReader::ProcessStopsAndBuses(std::vector<Node> stops_and_buses) {
		std::vector<Dict> stops, buses;
		for (const auto& object : stops_and_buses) {
			std::string obj_type;
			try {
				obj_type = object.AsMap().at("type").AsString();
			}
			catch (const std::exception&) {
				std::cerr << "Wrong data format"s << std::endl;
			}
			if (obj_type == "Stop") {
				stops.push_back(object.AsMap());
			}
			else if (obj_type == "Bus") {
				buses.push_back(object.AsMap());
			};
		};

		ParseStops(std::move(stops));
		ParseBuses(std::move(buses));
	}

	std::string ProcessColor(const Node& underlayer_color) {
		if (underlayer_color.IsString()) {
			return underlayer_color.AsString();
		}
		else {
			std::string color = underlayer_color.AsArray().size() == 3 ? "rgb("s : "rgba("s;
			bool first = true;
			for (const auto& node : underlayer_color.AsArray()) {
				if (!first) {
					color += ","s;
				}
				first = false;
				if (node.IsInt()) {
					color += std::to_string(node.AsInt());
				}
				else { //in case if number is double, for correct conversion to string is using stream
					std::stringstream stream;
					stream << node.AsDouble();
					std::string double_str = stream.str();
					color += double_str;
				};
			};
			color += ')';
			return color;
		};
	}

	void JsonReader::ProcessRenderSettings(render::MapRenderer& renderer, std::map<std::string, Node> render_settings) {
		//filling all MapRenderer settings
		renderer.SetWidthAndHeight(render_settings.at("width"s).AsDouble(), render_settings.at("height"s).AsDouble());
		renderer.SetPadding(render_settings.at("padding"s).AsDouble());
		renderer.SetStopRadius(render_settings.at("stop_radius"s).AsDouble());
		renderer.SetLineWidth(render_settings.at("line_width"s).AsDouble());
		renderer.SetBusAndStopsFontsSize(render_settings.at("bus_label_font_size"s).AsInt(), render_settings.at("stop_label_font_size"s).AsInt());
		//processing offset arrays with double
		std::vector<Node> bus_offset = render_settings.at("bus_label_offset").AsArray();
		std::vector<Node> stop_offset = render_settings.at("stop_label_offset").AsArray();
		renderer.SetBusAndStopLabelsOffset(bus_offset[0].AsDouble(), bus_offset[1].AsDouble(),
			stop_offset[0].AsDouble(), stop_offset[1].AsDouble());
		renderer.SetUnderlayerColor(ProcessColor(render_settings.at("underlayer_color")));
		renderer.SetUnderlayerWidth(render_settings.at("underlayer_width"s).AsDouble());

		//getting all colors as strings from array
		std::vector<std::string> color_palette;
		for (const auto& node : render_settings.at("color_palette").AsArray()) {
			color_palette.push_back(ProcessColor(node));
		};
		renderer.SetColorPalette(color_palette);
	}

	void JsonReader::ProcessRequests(std::vector<Node> requests) {
		for (const auto& node_request : requests) {
			//convert request type Node to map
			const auto& request_ = node_request.AsMap();
			//getting id of request, type of requested object and its name
			objects::Request request{};
			request.id = request_.at("id"s).AsInt();
			request.type = request_.at("type"s).AsString();
			if (request_.count("name"s)) { //map request doesnt have name
				request.name = request_.at("name"s).AsString();
			};
			//filling parsed requests
			parsed_requests.push_back(request);
		};
	}

	void JsonReader::ParseStops(std::vector<Dict> stops) {
		for (const auto& object : stops) {
			const std::string& stop_name = object.at("name"s).AsString();

			geo::Coordinates coordinates{};
			coordinates.lat = object.at("latitude"s).AsDouble();
			coordinates.lng = object.at("longitude"s).AsDouble();
			//adding stop to reader container
			parsed_stops_[stop_name] = coordinates;
			//adding road distances to reader container
			const std::map<std::string, Node>& distances = { object.at("road_distances"s).AsMap() };
			if (!distances.empty()) {
				for (const auto& [second_stop, length_as_node] : distances) {
					routes_lengths_[stop_name][second_stop] = length_as_node.AsInt();
				};
			};
		};
	}

	void JsonReader::ParseBuses(std::vector<Dict> buses) {
		for (const auto& object : buses) {
			const std::string& bus_name = object.at("name"s).AsString();
			auto& bus_route = parsed_buses_routes_[bus_name];

			bool is_roundtrip{ object.at("is_roundtrip"s).AsBool() };

			//filling pair of vector and bool is_roundtrip
			std::vector<std::string> stops;
			bus_route = std::make_pair(stops, is_roundtrip);

			//getting bus stops vector and converting stops names to strings
			const std::vector<Node>& bus_stops = object.at("stops"s).AsArray();
			for (const auto& stop : bus_stops) {
				bus_route.first.push_back(stop.AsString());
			};
			//check if route is not a roundtrip and and adding stops to route in reversed order if not
			if (!is_roundtrip) {
				if (bus_route.first.size() == 1) {
					bus_route.first.push_back(bus_route.first.front());
				}
				else {
					for (int i = int(parsed_buses_routes_.at(bus_name).first.size()) - 2; i >= 0; --i) {
						bus_route.first.push_back(bus_route.first[i]);
					};
				};
			};
		}
	}

}