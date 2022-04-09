#include <fstream>
#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

int main()
{
	std::ifstream i_file_stream("commands.txt");
	std::ofstream o_file_stream("result.json");

	transport::Catalogue catalogue;
	render::MapRenderer map_renderer;
	RequestHandler handler(i_file_stream, o_file_stream, catalogue, map_renderer);
	handler.ProcessAllRequests();

	return 0;
}