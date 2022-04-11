#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <map>
#include <sstream>
#include <set>
#include <algorithm>
#include <cmath>

#include "svg.h"
#include "domain.h"

using namespace std::string_literals;

namespace render {

	class MapRenderer {
	public:
		MapRenderer() = default;

		void SetWidthAndHeight(const double width, const double height);

		void SetPadding(const double padding);

		void SetStopRadius(const double stop_radius);

		void SetLineWidth(const double line_width);

		void SetBusAndStopsFontsSize(const int bus_label_font_size, const int stop_label_font_size);

		void SetBusAndStopLabelsOffset(const double bus_offset_x, const double bus_offset_y,
			const double stop_offset_x, const double stop_offset_y);

		void SetUnderlayerColor(const svg::Color& underlayer_color);

		void SetUnderlayerWidth(const double underlayer_width);

		void SetColorPalette(const std::vector<svg::Color> color_palette);

		void GetRoutes(const std::map<const objects::Bus*, std::vector<const objects::Stop*>>&);

		const std::string_view MapAsSvg();

	private:
		struct Settings {
			double height{}, width{}, padding{};
			double line_width{}, stop_radius{};
			int bus_label_font_size{}, stop_label_font_size{};
			svg::Point bus_label_offset{}, stop_label_offset{};
			svg::Color underlayer_color;
			double underlayer_width{};
			std::vector<svg::Color> color_palette;
		};

		Settings settings_;

		std::map<const objects::Bus*, std::vector<const objects::Stop*>, objects::BusPtrComp> routes_;
		std::map<const objects::Stop*, svg::Point, objects::StopPtrComp> unique_stops_with_xy_;
		std::string ready_map;

		double min_lng{}, min_lat{};
		double max_lng{}, max_lat{};

		//const objects::Stop* min_lng_stop, * max_lat_stop;
		double zoom_coef_;

		void FindMinMaxCoordinates();

		void CalculateZoomCoef();

		void GetXYCoordinates();

		double GetX(double lng);

		double GetY(double lat);

		void AddPolylines(svg::Document&);

		void AddRoutesNames(svg::Document&);

		void AddStopsCircles(svg::Document&);

		void AddStopsNames(svg::Document&);

	};

} //end of namespace render