#pragma once

/*
 *  Copyright (c) xEmbeddedTools team and contributors.
 *  Licensed under the Apache License, Version 2.0. See LICENSE file in the project root for details.
 */

// std
#include <array>
#include <cassert>
#include <span>
#include <string_view>

namespace xjson {
class Document
{
public:
    struct Value;
    struct Object;
    struct Array;

    struct Value
    {
        operator bool() const
        {
            return nullptr != this->p_begin && nullptr != this->p_end;
        }

        operator std::string_view() const
        {
            return { this->p_begin, this->p_end };
        }

        friend bool operator==(Value left_a, std::string_view right_a)
        {
            return std::string_view { left_a.p_begin, left_a.p_end } == right_a;
        }
        friend bool operator==(std::string_view left_a, Value right_a)
        {
            return std::string_view{ right_a.p_begin, right_a.p_end } == left_a;
        }

    private:
        Value() = default;
        Value(const char* p_begin_a, const char* p_end_a)
            : p_begin(p_begin_a)
            , p_end(p_end_a)
        {
        }

        const char* p_begin = nullptr;
        const char* p_end = nullptr;

        friend class Document;
    };
    struct Object
    {
        const std::size_t fields_count = 0u;

        template<typename Type> Type get(std::string_view key_a) const = delete;

        operator bool() const
        {
            return nullptr != this->p_begin && nullptr != this->p_end;
        }

    private:
        Object() = default;
        Object(std::size_t fields_count_a, const char* p_begin_a, const char* p_end_a)
            : fields_count(fields_count_a)
            , p_begin(p_begin_a)
            , p_end(p_end_a)
        {
        }

        const char* p_begin = nullptr;
        const char* p_end = nullptr;

        friend struct Array;
        friend class Document;
    };
    struct Array
    {
        const std::size_t elements_count = 0u;

        template<typename Type> Type get(std::size_t index_a) const = delete;

        operator bool() const
        {
            return nullptr != this->p_begin && nullptr != this->p_end;
        }

    private:
        Array() = default;
        Array(std::size_t elements_count_a, const char* p_begin_a, const char* p_end_a)
            : elements_count(elements_count_a)
            , p_begin(p_begin_a)
            , p_end(p_end_a)
        {
        }

        const char* p_begin = nullptr;
        const char* p_end = nullptr;

        friend struct Object;
        friend class Document;
    };

    Document(std::string_view document_data_a);

    template<typename Node> Node get_root() const = delete;

    bool is_valid() const
    {
        return this->valid;
    }

private:
    std::string_view data;
    bool valid = false;
};

template<> Document::Value Document::get_root() const;
template<> Document::Object Document::get_root() const;
template<> Document::Array Document::get_root() const;

template<> Document::Value Document::Object::get(std::string_view key_a) const;
template<> Document::Object Document::Object::get(std::string_view key_a) const;
template<> Document::Array Document::Object::get(std::string_view key_a) const;

template<> Document::Value Document::Array::get(std::size_t index_a) const;
template<> Document::Object Document::Array::get(std::size_t index_a) const;
template<> Document::Array Document::Array::get(std::size_t index_a) const;
} // namespace xjson