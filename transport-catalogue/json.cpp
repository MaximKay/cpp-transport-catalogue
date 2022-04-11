#include "json.h"

namespace json {

	using namespace std::string_literals;
	using namespace std::string_view_literals;
	namespace {
		Node LoadNumber(std::istream& input) {

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return Node(std::stoi(parsed_num));
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadString(std::istream& input) {
			std::string line;
			char c;
			while (input.get(c)) {
				if (c == '\\') {
					char next;
					input.get(next);
					if (next == '\"') {
						line += '\"';
					}
					else if (next == 'r') {
						line += '\r';
					}
					else if (next == 'n') {
						line += '\n';
					}
					else if (next == 't') {
						line += '\t';
					}
					else if (next == '\\') {
						line += '\\';
					};
				}
				else {
					if (c == '\"') {
						break;
					}
					line += c;
				};
			}
			if (c != '\"') {
				throw ParsingError("Failed to parse string node"s);
			};

			return Node(line);
		}

		Node LoadNode(std::istream& input);

		Node LoadArray(std::istream& input) {
			Array result;

			char c;
			std::string str;
			while (input >> c) {
				if (c == ']') {
					break;
				}
				else if (c == ',') {
					continue;
				}
				else {
					input.putback(c);
					result.push_back(LoadNode(input));
				};
			};
			if (c != ']') {
				throw ParsingError("Failed to parse array node"s);
			};

			return Node(move(result));
		}

		Node LoadDict(std::istream& input) {
			Dict result;

			char c;
			std::string key;
			while (input.get(c)) {
				if (c == '}') {
					break;
				}
				else if (c == ':') {
					//deleting unnessasary chars from front and back sides of the key
					const std::string prohibited_chars = " \r\n\t\"\\"s;
					while (prohibited_chars.find(key.front()) != std::string::npos) {
						key.erase(0, 1);
					};
					while (prohibited_chars.find(key.back()) != std::string::npos) {
						key.pop_back();
					};
					result[std::move(key)] = LoadNode(input);
				}
				else {
					if (c == ',') { continue; };
					key += c;
				};
			};
			if (c != '}') {
				throw ParsingError("Failed to parse map node"s);
			};

			return Node(move(result));
		}

		Node LoadBoolOrPtrCommand(std::istream& input) {
			std::string command;
			char c;
			while (input >> c) {
				if (c == ',') {
					break;
				}
				else if (c == ']' || c == '}') {
					input.putback(c);
					break;
				};
				command += c;
			};

			if (command == "true"s) {
				return Node{ true };
			}
			else if (command == "false"s) {
				return Node{ false };
			}
			else if (command == "null"s) {
				return Node{ nullptr };
			}
			else {
				throw ParsingError("Wrong command: "s + command);
			};
		}

		Node LoadNode(std::istream& input) {
			char c;
			input >> c;
			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n' || c == 't' || c == 'f') {
				input.putback(c);
				return LoadBoolOrPtrCommand(input);
			}
			else if (c == ']' || c == '}') {
				throw ParsingError("Wrong format of array or map"s);
			}
			else {
				input.putback(c);
				return LoadNumber(input);
			}
		}
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(*this);
	}

	bool Node::IsMap() const {
		return std::holds_alternative<Dict>(*this);
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(*this);
	}

	bool Node::IsDouble() const {
		return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(*this) && !std::holds_alternative<int>(*this);
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(*this);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(*this);
	}

	bool Node::IsNull() const {
		return std::holds_alternative<std::nullptr_t>(*this);
	}

	const Array& Node::AsArray() const {
		if (IsArray()) {
			return std::get<Array>(*this);
		}
		else {
			throw std::logic_error("Node data is not array"s);
		}
	}

	const Dict& Node::AsMap() const {
		if (IsMap()) {
			return std::get<Dict>(*this);
		}
		else {
			throw std::logic_error("Node data is not map"s);
		}
	}

	int Node::AsInt() const {
		if (IsInt()) {
			return std::get<int>(*this);
		}
		else {
			throw std::logic_error("Node data is not int"s);
		}
	}

	double Node::AsDouble() const {
		if (IsDouble()) {
			if (IsInt()) {
				return double(std::get<int>(*this));
			}
			else {
				return std::get<double>(*this);
			};
		}
		else { throw std::logic_error("Node data is not double"s); }
	}

	bool Node::AsBool() const {
		if (IsBool()) {
			return std::get<bool>(*this);
		}
		else {
			throw std::logic_error("Node data is not bool"s);
		}
	}

	const std::string& Node::AsString() const {
		if (IsString()) {
			return std::get<std::string>(*this);
		}
		else {
			throw std::logic_error("Node data is not string"s);
		}
	}

	const void* Node::AsNull() const {
		if (IsNull()) {
			return nullptr;
		}
		else {
			throw std::logic_error("Node data is not null"s);
		}
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(std::istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		auto& node = doc.GetRoot();
		if (node.IsArray()) {
			bool first = true;
			output << '[';
			for (const auto& obj : node.AsArray()) {
				if (first) {
					Print(Document{ obj }, output);
					first = false;
				}
				else {
					output << ", "sv;
					Print(Document{ obj }, output);
				};
			}
			output << ']';
		}
		else if (node.IsMap()) {
			bool first = true;
			output << '{';
			for (const auto& [key, node_] : node.AsMap()) {
				if (first) {
					output << '\"' << key << "\": "sv;
					Print(Document{ node_ }, output);
					first = false;
				}
				else {
					output << ", "sv;
					output << '\"' << key << "\": "sv;
					Print(Document{ node_ }, output);
				};
			}
			output << '}';
		}
		else if (node.IsInt()) {
			output << node.AsInt();
		}
		else if (node.IsDouble()) {
			output << node.AsDouble();
		}
		else if (node.IsPureDouble()) {
			output << node.AsDouble();
		}
		else if (node.IsBool()) {
			if (node.AsBool()) {
				output << "true"sv;
			}
			else {
				output << "false"sv;
			};
		}
		else if (node.IsString()) {
			std::string result{ "\""sv };
			for (const char c : node.AsString()) {
				if (c == '\\') {
					result += "\\\\"sv;
				}
				else if (c == '\"') {
					result += "\\\""sv;
				}
				else {
					result += c;
				};
			};
			result += "\""sv;
			output << result;
		}
		else if (node.IsNull()) {
			output << "null"sv;
		};
	}
}  // namespace json