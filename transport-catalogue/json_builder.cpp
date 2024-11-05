#include "json_builder.h"

#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json
{
    Builder::Builder() : root_(), nodes_stack_{ &root_ } {}

    Builder::KeyContext Builder::Key(std::string key)
    {
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );
        return KeyContext(*this);
    }

    Builder::Context Builder::Value(Node::Value value)
    {
        AddObject(std::move(value), true);
        return Context(*this);
    }

    Builder::DictContext Builder::StartDict()
    {
        AddObject(Dict{}, false);
        return DictContext(*this);
    }

    Builder::ArrayContext Builder::StartArray()
    {
        AddObject(Array{}, false);
        return ArrayContext(*this);
    }

    Builder& Builder::EndDict()
    {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside a dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray()
    {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside an array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build()
    {
        return root_;
    }

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }
        return nodes_stack_.back()->GetValue();
    }
    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
    }

    void Builder::AddObject(Node::Value value, bool one_shot)
    {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value))
        {
            Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        }
        else
        {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot)
            {
                nodes_stack_.pop_back();
            }
        }
    }


}
