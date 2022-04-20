#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	Node LoadNode(std::istream&);

	class Node final : 
        private std::variant<std::nullptr_t, Array, Dict, std::string_view, int, double, bool, std::string> {
	public: 
		using variant::variant;
		using Value = variant;

        bool IsInt() const {
            return std::holds_alternative<int>(*this);
        }
        int AsInt() const {
            using namespace std::literals;
            if (!IsInt()) {
                throw std::logic_error("Not an int"s);
            }
            return std::get<int>(*this);
        }

        bool IsPureDouble() const {
            return std::holds_alternative<double>(*this);
        }
        bool IsDouble() const {
            return IsInt() || IsPureDouble();
        }
        double AsDouble() const {
            using namespace std::literals;
            if (!IsDouble()) {
                throw std::logic_error("Not a double"s);
            }
            return IsPureDouble() ? std::get<double>(*this) : AsInt();
        }

        bool IsBool() const {
            return std::holds_alternative<bool>(*this);
        }
        bool AsBool() const {
            using namespace std::literals;
            if (!IsBool()) {
                throw std::logic_error("Not a bool"s);
            }

            return std::get<bool>(*this);
        }

        bool IsNull() const {
            return std::holds_alternative<std::nullptr_t>(*this);
        }

        bool IsArray() const {
            return std::holds_alternative<Array>(*this);
        }
        const Array& AsArray() const {
            using namespace std::literals;
            if (!IsArray()) {
                throw std::logic_error("Not an array"s);
            }

            return std::get<Array>(*this);
        }

        bool IsStringView() const {
            return std::holds_alternative<std::string_view>(*this);
        }
        const std::string_view& AsStringView() const {
            using namespace std::literals;
            if (!IsStringView()) {
                throw std::logic_error("Not a string_view"s);
            }

            return std::get<std::string_view>(*this);
        }

        bool IsString() const {
            return std::holds_alternative<std::string>(*this);
        }
        const std::string& AsString() const {
            using namespace std::literals;
            if (!IsString()) {
                throw std::logic_error("Not a string"s);
            }

            return std::get<std::string>(*this);
        }

        bool IsDict() const {
            return std::holds_alternative<Dict>(*this);
        }
        const Dict& AsDict() const {
            using namespace std::literals;
            if (!IsDict()) {
                throw std::logic_error("Not a dict"s);
            }

            return std::get<Dict>(*this);
        }

        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }

        const Value& GetValue() const {
            return *this;
        }

        Value& GetValue() {
            return *this;
        }
	};

	void PrintJson(const Node& node, std::ostream& output);

}  // namespace json