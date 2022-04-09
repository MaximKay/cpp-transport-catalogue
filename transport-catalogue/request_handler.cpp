#include "request_handler.h"

void RequestHandler::ProcessAllRequests() {
	json::JsonReader reader;
	reader.LoadData(input, renderer_);

	//adding bus stops to catalogue
	for (const auto& [name, coordinates] : reader.GetParsedStops()) {
		db_.AddStop(name, coordinates);
	};

	//when all stops added, we can add routes lengths
	db_.ParseRoutesLengths(reader.GetRoutesLengths());

	//adding buses information to catalogue 
	for (const auto& [bus, stops_and_bool] : reader.GetParsedBuses()) {
		//adding bus name and getting ptr to it
		const Bus* bus_ptr = db_.AddBus(bus, stops_and_bool.second);
		//adding routes for bus and buses for stop
		for (const auto& stop : stops_and_bool.first) {
			//getting stop pointer
			const Stop* stop_ptr = db_.FindStop(stop);
			//adding stop for bus
			db_.AddStopForBus(bus_ptr, stop_ptr);
			//adding bus for stop
			db_.AddBusForStop(stop_ptr, bus_ptr);
		};
	};

	//adding route data for buses
	db_.CalculateRoutesData();

	//vector of parsed requests
	const auto& requests = reader.GetParsedRequests();
	//vector of answers for requests
	std::vector<RequestAnswer> answers;
	
	//constructing answers for each request
	ProcessParsedStatRequests(requests, answers);

	//printing
	reader.Print(output, answers);
}

void RequestHandler::ProcessParsedStatRequests(const std::vector<Request>& requests, std::vector<RequestAnswer>& answers) {
	//temporary unordered map to save results, in order not to search again for the same requests
	//using request id as a map key
	std::unordered_map<int, RequestAnswer> temp_answers;
	for (const auto& request : requests) {
		//construct answer only if request hasnt been already processed
		if (temp_answers.count(request.id) == 0) {
			if (request.type == "Map"s) {
				temp_answers[request.id].id = request.id;
				renderer_.GetRoutes(db_.RoutesForMap());
				temp_answers[request.id].data = renderer_.MapAsSvg();
			}
			else {
				temp_answers[request.id] = db_.ConstructAnswerForRequest(request);
			};
		};
		//creating object for each request in answers vector
		answers.push_back(temp_answers.at(request.id));
	};
}