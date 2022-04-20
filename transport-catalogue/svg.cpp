#include "svg.h"

namespace svg {

	using namespace std::literals;

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;

		this->RenderAttrs(out);

		out << "/>"sv;
	}

	Polyline& Polyline::AddPoint(Point point) {
		Polyline::points_.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<polyline points=\""sv;
		bool first = true;
		for (const auto& point : Polyline::points_) {
			if (first) {
				out << point.x << ',' << point.y;
				first = false;
			}
			else {
				out << ' ' << point.x << ',' << point.y;
			};
		};
		out << "\""sv;

		this->RenderAttrs(out);

		out << "/>"sv;
	}

	Text& Text::SetPosition(Point pos) {
		text_coordinates_ = pos;
		return *this;
	}

	// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
	Text& Text::SetOffset(Point offset) {
		text_offset_ = offset;
		return *this;
	}

	// Задаёт размеры шрифта (атрибут font-size)
	Text& Text::SetFontSize(uint32_t size) {
		font_size_ = size;
		return *this;
	}

	// Задаёт название шрифта (атрибут font-family)
	Text& Text::SetFontFamily(std::string font_family) {
		font_family_ = font_family;
		return *this;
	}

	// Задаёт толщину шрифта (атрибут font-weight)
	Text& Text::SetFontWeight(std::string font_weight) {
		font_weight_ = font_weight;
		return *this;
	}

	// Задаёт текстовое содержимое объекта (отображается внутри тега text)
	Text& Text::SetData(std::string data) {
		data_ = data;
		return *this;
	}

	void Text::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<text";

		this->RenderAttrs(out);

		out << " x=\""sv << text_coordinates_.x << "\" y=\""sv << text_coordinates_.y << "\" "sv <<
			"dx=\""sv << text_offset_.x << "\" dy=\""sv << text_offset_.y << "\" font-size=\""sv <<
			font_size_ << "\""sv;
		if (font_family_) {
			out << " font-family=\""sv << *font_family_ << "\""sv;
		};
		if (font_weight_) {
			out << " font-weight=\""sv << *font_weight_ << "\""sv;
		};
		out << ">"sv;
		for (const char c : data_) {
			if (c == '\"') {
				out << "&quot;";
			}
			else if (c == '\'') {
				out << "&apos;";
			}
			else if (c == '<') {
				out << "&lt;";
			}
			else if (c == '>') {
				out << "&gt;";
			}
			else if (c == '&') {
				out << "&amp;";
			}
			else {
				out << c;
			};
		};

		out << "</text>"sv;
	}

	// Добавляет в svg-документ объект-наследник svg::Object
	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_.push_back(std::move(obj));
	}

	// Выводит в ostream svg-представление документа
	void Document::Render(std::ostream& out) const {

		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		for (const auto& obj : objects_) {
			out << ' '; //space between objects
			obj->Render(out);
		};

		out << "</svg>"sv;
	}

}  // namespace svg