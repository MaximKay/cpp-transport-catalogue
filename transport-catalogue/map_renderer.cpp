#include "map_renderer.h"

namespace render {

		void MapRenderer::SetWidthAndHeight(const double width, const double height) {
			width_ = width;
			height_ = height;
		}

		void MapRenderer::SetPadding(const double padding) { padding_ = padding; }
		void MapRenderer::SetStopRadius(const double stop_radius) { stop_radius_ = stop_radius; }
		void MapRenderer::SetLineWidth(const double line_width) { line_width_ = line_width; }

		void MapRenderer::SetBusAndStopsFontsSize(const int bus_label_font_size, const int stop_label_font_size) {
			bus_label_font_size_ = bus_label_font_size;
			stop_label_font_size_ = stop_label_font_size;
		}

		void MapRenderer::SetBusAndStopLabelsOffset(const double bus_offset_x, const double bus_offset_y,
			const double stop_offset_x, const double stop_offset_y) {
			bus_label_offset_.x = bus_offset_x;
			bus_label_offset_.y = bus_offset_y;
			stop_label_offset_.x = stop_offset_x;
			stop_label_offset_.y = stop_offset_y;
		}

		void MapRenderer::SetUnderlayerColor(const svg::Color& underlayer_color) {
			underlayer_color_ = underlayer_color;
		}

		void MapRenderer::SetUnderlayerWidth(const double underlayer_width) { underlayer_width_ = underlayer_width; }
		void MapRenderer::SetColorPalette(const std::vector<svg::Color> color_palette) { color_palette_ = color_palette; }

		void MapRenderer::GetRoutes(const std::map<const objects::Bus*, std::vector<const objects::Stop*>>& routes) {
			//creating maps of sorted routes and unique stops
			for (const auto& [bus, stops] : routes) {
				routes_[bus] = stops;

				for (const objects::Stop* stop : stops) {
					unique_stops_with_xy_[stop];
				};
			};
		}

		const std::string_view MapRenderer::MapAsSvg() {
			FindMinMaxCoordinates();
			CalculateZoomCoef();
			GetXYCoordinates();

			svg::Document doc; //create and fill doc of svg objects
			AddPolylines(doc);
			AddRoutesNames(doc);
			AddStopsCircles(doc);
			AddStopsNames(doc);

			std::stringstream stream;
			doc.Render(stream);

			ready_map = stream.str();

			return ready_map;
		}

		void MapRenderer::FindMinMaxCoordinates() {
			for (const auto& [stop, _] : unique_stops_with_xy_) {
				const auto& longitude = stop->point_.lng;
				const auto& latitude = stop->point_.lat;
				if (min_lng == 0 || longitude < min_lng) { min_lng = longitude; };
				if (min_lat == 0 || latitude < min_lat) { min_lat = latitude; };
				if (max_lng == 0 || longitude > max_lng) { max_lng = longitude; };
				if (max_lat == 0 || latitude > max_lat) { max_lat = latitude; };
			};
		}

		void MapRenderer::CalculateZoomCoef() {
			double zoom_coef{};
			double width_zoom_coef = (width_ - 2 * padding_) / (max_lng - min_lng);
			double height_zoom_coef = (height_ - 2 * padding_) / (max_lat - min_lat);
			zoom_coef = std::min(width_zoom_coef, height_zoom_coef);
			if (zoom_coef == 0) { //if min is zero, then choose another(max)
				zoom_coef = std::max(width_zoom_coef, height_zoom_coef);
			};
			zoom_coef_ = zoom_coef;
		}

		void MapRenderer::GetXYCoordinates() {
			for (auto& [stop_ptr, point] : unique_stops_with_xy_) { //converting lng & lat to x & y
				svg::Point point_xy(GetX(stop_ptr->point_.lng), GetY(stop_ptr->point_.lat));
				point = point_xy;
			};
		}

		double MapRenderer::GetX(double lng) { //converting longitude to X
			return (lng - min_lng) * zoom_coef_ + padding_;
		}

		double MapRenderer::GetY(double lat) { //converting latitude to Y
			return (max_lat - lat) * zoom_coef_ + padding_;
		}

		void MapRenderer::AddPolylines(svg::Document& doc) {
			int color{}; //color palette index 
			for (const auto& [bus_ptr, stops] : routes_) {
				if (!stops.empty()) {  //if current route has zero stops skip it

					svg::Polyline polyline; //create polyline object and setting its properties
					polyline.SetStrokeColor(color_palette_[color]).SetFillColor(svg::NoneColor).SetStrokeWidth(line_width_).
						SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

					for (const auto& stop : stops) { //adding all points from one bus route
						polyline.AddPoint(unique_stops_with_xy_.at(stop));
					};
					doc.Add(polyline); //adding ready polyline to doc

					if (color == (color_palette_.size() - 1)) { //if color is last - going to the first color
						color = 0;
					}
					else {
						++color;
					};
				};
			};
		}

		void MapRenderer::AddRoutesNames(svg::Document& doc) {
			int color{}; //color palette index 
			for (const auto& [bus_ptr, stops] : routes_) {
				if (!stops.empty()) {  //if current route has zero stops skip it
					const svg::Point& first_stop_xy = unique_stops_with_xy_.at(stops.front());

					//create text object and setting its properties
					svg::Text text;
					text.SetFontSize(bus_label_font_size_).SetFillColor(color_palette_[color]).SetFontFamily("Verdana"s).
						SetFontWeight("bold"s).SetPosition(first_stop_xy).SetOffset(bus_label_offset_).SetData(bus_ptr->bus_name_);
					//create underlayer object and setting its properties
					svg::Text underlayer(text);
					underlayer.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_).SetStrokeWidth(underlayer_width_).
						SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

					doc.Add(underlayer); //add objects for the first stop of the route
					doc.Add(text);

					if (bus_ptr->is_roundtrip_ == false) { //if not a roundtrip, add same objects for the last stop
						//not a roundtrip route always will have odd number of stops, 
						//so last stop will have index .size()/2
						const svg::Point& last_stop_xy = unique_stops_with_xy_.at(stops[stops.size() / 2]);
						if (stops[stops.size() / 2] != stops.front()) { //stops must be different
							underlayer.SetPosition(last_stop_xy);
							text.SetPosition(last_stop_xy);
							doc.Add(underlayer); //add objects for the last stop of the route
							doc.Add(text);
						}
					};

					if (color == (color_palette_.size() - 1)) { //if color is last - going to the first color
						color = 0;
					}
					else {
						++color;
					};
				};
			};
		}

		void MapRenderer::AddStopsCircles(svg::Document& doc) {
			for (const auto& [stop, point] : unique_stops_with_xy_) {
				svg::Circle circle;
				circle.SetCenter(point).SetRadius(stop_radius_).SetFillColor("white"s);
				doc.Add(circle);
			};
		}

		void MapRenderer::AddStopsNames(svg::Document& doc) {
			for (const auto& [stop, point] : unique_stops_with_xy_) {

				//create text object and setting its properties
				svg::Text text;
				text.SetFontSize(stop_label_font_size_).SetFillColor("black"s).SetFontFamily("Verdana"s).
					SetPosition(point).SetOffset(stop_label_offset_).SetData(stop->stop_);
				//create underlayer object and setting its properties
				svg::Text underlayer(text);
				underlayer.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_).SetStrokeWidth(underlayer_width_).
					SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				doc.Add(underlayer);
				doc.Add(text);

			};
		}

} //end of namespace render