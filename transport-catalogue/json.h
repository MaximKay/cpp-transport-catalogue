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

	class Node {
	public:
		Node() = default;
		Node(Array array) : node_(std::move(array)) {}
		Node(Dict map) : node_(std::move(map)) {}
		Node(int value) : node_(value) {}
		Node(double value) : node_(value) {}
		Node(bool value) : node_(value) {}
		Node(std::string str) : node_(str) {}
		Node(void*) : node_() {}

		bool operator==(const Node& other) const {
			return other.node_ == node_;
		}

		bool operator!=(const Node& other) const {
			return !(other.node_ == node_);
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

	private:
		std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string> node_;
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