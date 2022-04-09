#pragma once
#include <iostream>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

class RequestHandler {
public:
	RequestHandler() = delete;

	RequestHandler(transport::Catalogue& db, render::MapRenderer& renderer) : db_(db), renderer_(renderer){
	}

	RequestHandler(std::istream& in, std::ostream& out, transport::Catalogue& db, render::MapRenderer& renderer) 
		: db_(db), renderer_(renderer), input(in), output(out) {
	}

	void ProcessAllRequests();

	void ProcessParsedStatRequests(const std::vector<Request>&, std::vector<RequestAnswer>&);

private:
	transport::Catalogue& db_;
	render::MapRenderer& renderer_;
	std::istream& input = std::cin;
	std::ostream& output = std::cout;
};