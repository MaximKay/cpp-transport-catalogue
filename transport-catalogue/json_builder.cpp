#include "json_builder.h"

namespace json {

	using namespace std::string_literals;

	//start of Builder class methods & objects
	Builder::KeyItemContext Builder::Key(std::string str) {
		key = str;
		return KeyItemContext{ *this };
	}

	Builder::DictItemContext Builder::ValueAfterKey(Node::Value value) {
		Node node = { std::visit([](const auto& val) { return Node{val}; }, value) };

		auto& dict_node = nodes_stack_.back()->GetValue();
		std::get<Dict>(dict_node)[*key] = node;
		key.reset();

		return DictItemContext{ *this };
	}

	Builder::ArrayItemContext Builder::ValueInArray(Node::Value value) {
		Node node = { std::visit([](const auto& val) { return Node{val}; }, value) };

		auto& array_ref = nodes_stack_.back()->GetValue();
		std::get<Array>(array_ref).push_back(node);

		return ArrayItemContext{ *this };
	}

	Builder::Context Builder::Value(Node::Value value) {
		Node node = { std::visit([](const auto& val) { return Node{val}; }, value) };
		empty_node = false;

		if (construct_finished) {
			throw std::logic_error("Constructing already finished!"s);
		}
		else if (nodes_stack_.empty()) {
			node_ = node;
			construct_finished = true;
		}
		else {
			if (nodes_stack_.back()->IsArray()) {
				auto& array_ref = nodes_stack_.back()->GetValue();
				std::get<Array>(array_ref).push_back(node);
			}
			else if (nodes_stack_.back()->IsDict()) {
				if (key != std::nullopt) {
					auto& dict_node = nodes_stack_.back()->GetValue();
					std::get<Dict>(dict_node)[*key] = node;
					key.reset();
				}
				else {
					throw std::logic_error("Key must be setted!"s);
				};
			};
		};

		return Context{ *this };
	}

	Builder::DictItemContext Builder::StartDict() {
		Node dict_node{ Dict{} };
		empty_node = false;

		if (construct_finished) {
			throw std::logic_error("Constructing already finished!"s);
		}
		else if (nodes_stack_.empty()) {
			node_ = dict_node;
			nodes_stack_.push_back(&node_);
		}
		else {
			if (nodes_stack_.back()->IsDict()) {
				if (key != std::nullopt) {
					auto& dict_node_ref = nodes_stack_.back()->GetValue();
					auto& ref_to_val = std::get<Dict>(dict_node_ref)[*key];
					ref_to_val = dict_node;
					nodes_stack_.push_back(&ref_to_val);
					key.reset();
				}
				else {
					throw std::logic_error("Key must be setted!"s);
				};
			}
			else if (nodes_stack_.back()->IsArray()) {
				auto& array_ref = nodes_stack_.back()->GetValue();
				auto& placed_dict_ref = std::get<Array>(array_ref).emplace_back(dict_node);
				nodes_stack_.push_back(&placed_dict_ref);
			};
		};

		return DictItemContext{ *this };
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("Dictionary is not started!"s);
		}
		else if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Current container is not a dictionary!"s);
		}
		else {
			nodes_stack_.pop_back();
			if (nodes_stack_.empty()) {
				construct_finished = true;
			};
		};

		return *this;
	}

	Builder::ArrayItemContext Builder::StartArray() {
		Node array_node{ Array{} };
		empty_node = false;

		if (construct_finished) {
			throw std::logic_error("Constructing already finished!"s);
		}
		else if (nodes_stack_.empty()) {
			node_ = array_node;
			nodes_stack_.push_back(&node_);
		}
		else {
			if (nodes_stack_.back()->IsDict()) {
				if (key != std::nullopt) {
					auto& dict_node = nodes_stack_.back()->GetValue();
					auto& ref_to_val = std::get<Dict>(dict_node)[*key];
					ref_to_val = array_node;
					nodes_stack_.push_back(&ref_to_val);
					key.reset();
				}
				else {
					throw std::logic_error("Key must be setted!"s);
				};
			}
			else if (nodes_stack_.back()->IsArray()) {
				auto& array_ref = nodes_stack_.back()->GetValue();
				auto& placed_array_ref = std::get<Array>(array_ref).emplace_back(array_node);
				nodes_stack_.push_back(&placed_array_ref);
			};
		};

		return ArrayItemContext{ *this };
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("Array is not started!"s);
		}
		else if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Current container is not an array!"s);
		}
		else {
			nodes_stack_.pop_back();
			if (nodes_stack_.empty()) {
				construct_finished = true;
			};
		};

		return *this;
	}

	Node Builder::Build() {
		if (empty_node == true) {
			throw std::logic_error("Node is empty!"s);
		}
		else if (!nodes_stack_.empty() || construct_finished == false) {
			throw std::logic_error("Constructing is not finished!"s);
		};

		return node_;
	}

}//end of namespace json