#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <optional>
#include "json.h"

namespace json {

    class Builder
    {
    public:
        Builder();
        class Context;

        class KeyContext;
        class DictContext;
        class ArrayContext;

        class Context
        {
        public:
            explicit Context(Builder& builder) : builder_(builder) {};
            KeyContext Key(const std::string& key)
            {
                return builder_.Key(key);
            }
            Context Value(Node::Value value)
            {
                return *this;
            }
            DictContext StartDict()
            {
                return builder_.StartDict();
            }
            Builder& EndDict()
            {
                return builder_.EndDict();
            }
            ArrayContext StartArray()
            {
                return builder_.StartArray();
            }
            Builder& EndArray()
            {
                return builder_.EndArray();
            }

            Node Build()
            {
                return builder_.root_;
            }

        private:
            Builder& builder_;
        };
        class DictContext : public Context
        {
        public:
            explicit DictContext(Builder& builder) : Context(builder) {};

            Context Value(Node::Value value) = delete;
            DictContext StartDict() = delete;
            ArrayContext StartArray() = delete;
            Builder& EndArray() = delete;
            Node Build() = delete;
        };


        class KeyContext : public Context
        {
        public:
            explicit KeyContext(Builder& builder) : Context(builder), builder_(builder) {};

            DictContext Value(Node::Value value)
            {
                builder_.Value(value);
                return DictContext(builder_);
            }

            KeyContext Key(const std::string& key) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
            Node Build() = delete;
        private:
            Builder& builder_;
        };


        class ArrayContext : public Context
        {
        public:
            explicit ArrayContext(Builder& builder) : Context(builder), builder_(builder) {};

            ArrayContext Value(Node::Value value)
            {
                builder_.Value(value);
                return ArrayContext(builder_);
            }

            KeyContext Key(const std::string& key) = delete;
            Builder& EndDict() = delete;
            Node Build() = delete;
        private:
            Builder& builder_;
        };

        KeyContext Key(std::string key);
        Context Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;

        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);
    };

} // namespace json
