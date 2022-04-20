#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <variant>
#include <optional>

#include "json.h"

namespace json {

	class Builder {
	public:
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;

		class Context {
		public:
			Context(Builder& ref) : builder(ref) {
			}

			KeyItemContext Key(std::string str) {
				return builder.Key(str);
			}

			Builder& EndDict() {
				return builder.EndDict();
			}

			Context Value(Node::Value value) {
				return builder.Value(value);
			}

			DictItemContext StartDict() {
				return builder.StartDict();
			}

			ArrayItemContext StartArray() {
				return builder.StartArray();
			}

			Builder& EndArray() {
				return builder.EndArray();
			}

			Node Build() {
				return builder.Build();
			}

			Builder& builder;
		};

		//supporting class for dict items to prevent calling of wrong methods
		class DictItemContext : public Context {
		public:
			DictItemContext(Builder& builder_ref) : Context(builder_ref) {
			}

			Context Value(Node::Value) = delete;

			DictItemContext StartDict() = delete;

			ArrayItemContext StartArray() = delete;

			Builder& EndArray() = delete;

			Node Build() = delete;
		};

		//supporting class for array items to prevent calling of wrong methods
		class ArrayItemContext : public Context {
		public:
			ArrayItemContext(Builder& builder_ref) : Context(builder_ref) {
			}

			ArrayItemContext Value(Node::Value value) {
				return builder.ValueInArray(value);
			}

			KeyItemContext Key(std::string str) = delete;

			Builder& EndDict() = delete;

			Node Build() = delete;
		};

		//supporting class for key items to prevent calling of wrong methods
		class KeyItemContext : public Context {
		public:
			KeyItemContext(Builder& builder_ref) : Context(builder_ref) {
			}

			DictItemContext Value(Node::Value value) {
				return builder.ValueAfterKey(value);
			}

			KeyItemContext Key(std::string str) = delete;

			Builder& EndDict() = delete;

			Builder& EndArray() = delete;

			Node Build() = delete;
		};

		//start of Builder class methods & objects
		KeyItemContext Key(std::string);

		DictItemContext ValueAfterKey(Node::Value);

		ArrayItemContext ValueInArray(Node::Value);

		Context Value(Node::Value);

		DictItemContext StartDict();

		Builder& EndDict();

		ArrayItemContext StartArray();

		Builder& EndArray();

		Node Build();

	private:
		Node node_;
		std::vector<Node*> nodes_stack_;
		std::optional<std::string> key;
		bool construct_finished{ false }, empty_node{ true };
	};

}//end of namespace json