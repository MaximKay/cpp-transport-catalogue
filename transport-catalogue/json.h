#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cmath>
#include <cstddef>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node final : std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string> {
	public:
		using variant::variant;

		bool operator==(const Node& other) const {
			return *this == other;
		}

		bool operator!=(const Node& other) const {
			return !(*this == other);
		}

		bool IsArray() const;
		bool IsMap() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;

		const Array& AsArray() const;
		const Dict& AsMap() const;
		int AsInt() const;
		double AsDouble() const;
		bool AsBool() const;
		const std::string& AsString() const;
		const void* AsNull() const;
	};

	class Document {
	public:
		Document(Node root) : root_(std::move(root)) {
		}

		bool operator==(const Document& other) {
			return root_ == other.root_;
		}

		bool operator!=(const Document& other) {
			return !(root_ == other.root_);
		}

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json