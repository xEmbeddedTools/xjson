// this
#include <xjson/Document.hpp>

#ifndef XJSON_MAX_NESTING_DEPTH
#define XJSON_MAX_NESTING_DEPTH 17u
#endif

namespace {
struct Lexeme
{
    enum class Kind : std::uint8_t
    {
        unknown,
        string,
        number,
        keyword,
        separator
    };

    using enum Kind;

    Kind kind = unknown;
    std::string_view value;
};
struct Scope
{
    enum class Kind : std::uint32_t
    {
        unknown,
        object,
        array
    };

    using enum Kind;

    std::string_view push;
    std::string_view pop;
    Kind kind = unknown;
};
struct Transition
{
    std::string_view value;
    Lexeme::Kind trigger = Lexeme::unknown;
    Scope::Kind scope = Scope::unknown;

    std::span<const std::uint8_t> next;
};
struct TransitionCallback
{
    using Function =
        bool (*)(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind current_scope_a, const Lexeme& lexeme_a, void* p_user_data_a);

    Function function = nullptr;
    void* p_user_data = nullptr;
};

template<std::uint8_t width, std::size_t capacity> class BitStack
{
public:
    template<typename T> void push(T v)
    {
        const std::uint32_t masked = static_cast<std::uint32_t>(v) & mask;
        this->d[size / 32u] |= (masked << (size & 31u));
        size += width;
    }

    void pop()
    {
        if (size >= width)
        {
            size -= width;
            this->d[size / 32u] &= ~(mask << (size & 31u));
        }
    }

    template<typename T> T top() const
    {
        const std::uint32_t current = size > 0u ? size - width : 0u;
        return static_cast<T>((this->d[current / 32u] >> (current & 31u)) & mask);
    }

    bool is_empty() const
    {
        return 0u == size;
    }

    std::size_t get_size() const
    {
        return this->size / width;
    }

    bool is_full() const
    {
        return this->get_size() == capacity;
    }

private:
    std::uint32_t size = 0u;
    std::uint32_t d[((capacity * width) + 31u) / 32u] = { 0u };
    static constexpr std::uint32_t mask = (32u == width) ? 0xFFFFFFFFu : ((1u << width) - 1u);
};

constexpr std::uint8_t value_in_array_nexts[] { 10u, 17u };
constexpr std::uint8_t separator_in_array_nexts[] { 0u, 11u, 12u, 13u, 14u, 15u, 16u };
constexpr std::uint8_t array_begin_nexts[] { 0u, 11u, 12u, 13u, 14u, 15u, 16u, 17 };
constexpr std::uint8_t array_end_nexts[] { 1u, 2u, 10u };
constexpr std::uint8_t object_end_nexts[] { 1u, 2u, 10u, 17u };
constexpr std::uint8_t separator_in_object_nexts[] { 8u };
constexpr std::uint8_t value_in_object_nexts[] { 1u, 2u };
constexpr std::uint8_t assigment_nexts[] { 0u, 3u, 4u, 5u, 6u, 16u };
constexpr std::uint8_t key_nexts[] { 9u };
constexpr std::uint8_t object_begin_nexts[] { 1u, 8u };
constexpr std::uint8_t root_nexts[] { 0u, 16u, 18u, 19u, 20u, 21u, 22u };

constexpr std::array transitions { Transition { .value = "{", .trigger = Lexeme::separator, .scope = Scope::object, .next = object_begin_nexts },
                                   Transition { .value = "}", .trigger = Lexeme::separator, .scope = Scope::object, .next = object_end_nexts },
                                   Transition { .value = ",", .trigger = Lexeme::separator, .scope = Scope::object, .next = separator_in_object_nexts },
                                   Transition { .value = "", .trigger = Lexeme::string, .scope = Scope::object, .next = value_in_object_nexts },
                                   Transition { .value = "", .trigger = Lexeme::number, .scope = Scope::object, .next = value_in_object_nexts },
                                   Transition { .value = "true", .trigger = Lexeme::keyword, .scope = Scope::object, .next = value_in_object_nexts },
                                   Transition { .value = "false", .trigger = Lexeme::keyword, .scope = Scope::object, .next = value_in_object_nexts },
                                   Transition { .value = "null", .trigger = Lexeme::keyword, .scope = Scope::object, .next = value_in_object_nexts },
                                   Transition { .value = "", .trigger = Lexeme::string, .scope = Scope::object, .next = key_nexts },
                                   Transition { .value = ":", .trigger = Lexeme::separator, .scope = Scope::object, .next = assigment_nexts },

                                   Transition { .value = ",", .trigger = Lexeme::separator, .scope = Scope::array, .next = separator_in_array_nexts },
                                   Transition { .value = "", .trigger = Lexeme::string, .scope = Scope::array, .next = value_in_array_nexts },
                                   Transition { .value = "true", .trigger = Lexeme::keyword, .scope = Scope::array, .next = value_in_array_nexts },
                                   Transition { .value = "false", .trigger = Lexeme::keyword, .scope = Scope::array, .next = value_in_array_nexts },
                                   Transition { .value = "null", .trigger = Lexeme::keyword, .scope = Scope::array, .next = value_in_array_nexts },
                                   Transition { .value = "", .trigger = Lexeme::number, .scope = Scope::array, .next = value_in_array_nexts },
                                   Transition { .value = "[", .trigger = Lexeme::separator, .scope = Scope::array, .next = array_begin_nexts },
                                   Transition { .value = "]", .trigger = Lexeme::separator, .scope = Scope::array, .next = array_end_nexts },

                                   Transition { .value = "", .trigger = Lexeme::string, .scope = Scope::unknown, .next = {} },
                                   Transition { .value = "", .trigger = Lexeme::number, .scope = Scope::unknown, .next = {} },
                                   Transition { .value = "true", .trigger = Lexeme::keyword, .scope = Scope::unknown, .next = {} },
                                   Transition { .value = "false", .trigger = Lexeme::keyword, .scope = Scope::unknown, .next = {} },
                                   Transition { .value = "null", .trigger = Lexeme::keyword, .scope = Scope::unknown, .next = {} },

                                   Transition { .value = "", .scope = Scope::unknown, .next = root_nexts } };

static Scope object_scope { .push = "{", .pop = "}", .kind = Scope::object };
static Scope array_scope { .push = "[", .pop = "]", .kind = Scope::array };

Lexeme evaluate_next_lexeme(std::string_view json_data_a)
{
    enum Flag : std::uint32_t
    {
        in_string = 0x1u,
        has_data = 0x2u,
        escape_character = 0x4u,
        dirty = 0x8u
    };

    auto resolve_kind = [](std::string_view value_a) {
        if ("true" == value_a || "false" == value_a || "null" == value_a)
        {
            return Lexeme::keyword;
        }

        std::size_t fp_pos = value_a.length();

        for (std::size_t i = 0; i < value_a.length(); i++)
        {
            if ('.' == value_a[i])
            {
                if (value_a.length() == fp_pos)
                {
                    fp_pos = i;
                }
                else
                {
                    return Lexeme::unknown;
                }
            }
        }

        const std::string_view natural_part = { value_a.begin() + ('-' == value_a.front() ? 1u : 0u), value_a.begin() + fp_pos };

        if (false == natural_part.empty())
        {
            if (natural_part.front() >= '0' && natural_part.front() <= '9' && value_a.length() < 32u)
            {
                std::uint32_t zeros_mask = 0x0u;
                std::uint32_t non_zeros_mask = 0x0u;

                for (std::size_t i = 0; i < natural_part.length(); i++)
                {
                    char c = natural_part[i];

                    if ('0' == c)
                    {
                        zeros_mask |= 1u << i;
                    }
                    else
                    {
                        non_zeros_mask |= 1u << i;
                    }

                    if (c < '0' || c > '9')
                    {
                        return Lexeme::unknown;
                    }
                }

                if ((0x1u == (zeros_mask & 0x1u) && 0x0u != non_zeros_mask) || (0u == non_zeros_mask && 0x3u == (zeros_mask & 0x3u)))
                {
                    return Lexeme::unknown;
                }
            }
            else
            {
                return Lexeme::unknown;
            }
        }
        else
        {
            return Lexeme::unknown;
        }

        if (fp_pos != value_a.length())
        {
            const std::string_view fractional_part = { value_a.begin() + fp_pos + 1u, value_a.end() };

            if (false == fractional_part.empty())
            {
                for (const auto c : fractional_part)
                {
                    if (c < '0' || c > '9')
                    {
                        return Lexeme::unknown;
                    }
                }
            }
            else
            {
                return Lexeme::unknown;
            }
        }

        return Lexeme::number;
    };

    const char* p_begin = json_data_a.data();
    const char* p_current = json_data_a.data();
    const char* const p_end = json_data_a.data() + json_data_a.size();

    std::uint16_t start_column = 0u;
    std::uint32_t flags = 0x0u;

    while (p_current != p_end)
    {
        char c = *p_current;

        if (in_string == (flags & in_string))
        {
            if ('\"' == c)
            {
                flags &= ~(has_data | in_string);
                return { .kind = Lexeme::string, .value = std::string_view(p_begin, p_current - p_begin) };
            }
            else
            {
                p_current++;
            }
        }
        else
        {
            switch (c)
            {
                case '\"': {
                    flags |= (in_string | has_data);
                    p_current++;
                    p_begin = p_current;
                }
                break;

                case ' ':
                case '\r':
                case '\t':
                case '\n': {
                    if (has_data == (flags & has_data))
                    {
                        flags = (flags & ~has_data) | dirty;

                        const std::string_view value = std::string_view(p_begin, p_current - p_begin);
                        return { .kind = resolve_kind(value), .value = value };
                    }

                    p_current++;
                }
                break;

                case '[':
                case ']':
                case '{':
                case '}':
                case ':':
                case ',': {
                    if (has_data == (flags & has_data))
                    {
                        flags &= ~has_data;

                        const std::string_view value = std::string_view(p_begin, p_current - p_begin);
                        return { .kind = resolve_kind(value), .value = value };
                    }

                    return { .kind = Lexeme::separator, .value = std::string_view(p_current, 1) };
                }
                break;

                default: {
                    if (has_data != (flags & has_data))
                    {
                        p_begin = p_current;
                        flags |= has_data;
                    }
                    p_current++;
                }
            }
        }
    }

    if (has_data == (flags & has_data))
    {
        const std::string_view value = std::string_view(p_begin, p_current - p_begin);
        return { .kind = resolve_kind(value), .value = value };
    }

    return { .kind = Lexeme::unknown, .value = "" };
}

bool evaluate_json(std::string_view json_data_a, TransitionCallback on_transition_a = {})
{
    BitStack<2u, XJSON_MAX_NESTING_DEPTH> context;
    std::size_t current_transition = transitions.size() - 1u;
    const char* p_current = json_data_a.data();
    const char* p_end = json_data_a.data() + json_data_a.size();

    while (p_current != p_end)
    {
        const auto lexeme = evaluate_next_lexeme({ p_current, p_end });

        if (Lexeme::unknown != lexeme.kind)
        {
            auto f = [&](std::size_t index_a) -> bool {
                if (transitions[index_a].scope == context.top<Scope::Kind>() && transitions[index_a].trigger == lexeme.kind)
                {
                    if (false == transitions[index_a].value.empty())
                    {
                        return transitions[index_a].value == lexeme.value;
                    }
                    return true;
                }
                return false;
            };

            if (object_scope.push == lexeme.value)
            {
                if (false == context.is_full())
                {
                    context.push(object_scope.kind);
                }
                else
                {
                    return false;
                }
            }
            else if (array_scope.push == lexeme.value)
            {
                if (false == context.is_full())
                {
                    context.push(array_scope.kind);
                }
                else
                {
                    return false;
                }
            }

            const auto next_node_itr = std::find_if(transitions[current_transition].next.begin(), transitions[current_transition].next.end(), f);
            if (next_node_itr != transitions[current_transition].next.end())
            {
                current_transition = *next_node_itr;

                if (nullptr != on_transition_a.function)
                {
                    if (false ==
                        on_transition_a.function(current_transition, context.get_size(), context.top<Scope::Kind>(), lexeme, on_transition_a.p_user_data))
                    {
                        return true;
                    }
                }
            }
            else
            {
                return false;
            }

            if (object_scope.pop == lexeme.value)
            {
                if (false == context.is_empty() && Scope::object == context.top<Scope::Kind>())
                {
                    context.pop();
                }
                else
                {
                    return false;
                }
            }
            else if (array_scope.pop == lexeme.value)
            {
                if (false == context.is_empty() && Scope::array == context.top<Scope::Kind>())
                {
                    context.pop();
                }
                else
                {
                    return false;
                }
            }

            p_current = lexeme.value.data() + lexeme.value.length() + (lexeme.kind == Lexeme::string ? 1u : 0u);
        }
        else
        {
            if (false == lexeme.value.empty())
            {
                return false;
            }
            else
            {
                p_current = p_end;
            }
        }
    }

    return 0u == context.get_size();
}

struct FindKeyContext
{
    std::size_t found_cnt = 0u;
    std::string_view key;

    const char* p_current = nullptr;
};
bool find_key(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind, const Lexeme& lexeme_a, void* p_user_data_a)
{
    auto* p_context = reinterpret_cast<FindKeyContext*>(p_user_data_a);

    if (2u == p_context->found_cnt)
    {
        if (0u == current_transition_a || 3u == current_transition_a || 16u == current_transition_a)
        {
            p_context->p_current = Lexeme::string == lexeme_a.kind ? lexeme_a.value.data() - 1u : lexeme_a.value.data();
            return false;
        }
        else
        {
            p_context->found_cnt = 0u;
        }
    }

    if (1u == p_context->found_cnt)
    {
        if (9u == current_transition_a)
        {
            p_context->found_cnt = 2u;
        }
        else
        {
            p_context->found_cnt = 0u;
        }
    }

    if (0u == p_context->found_cnt && 8u == current_transition_a && p_context->key == lexeme_a.value && 1u == context_size_a)
    {
        p_context->found_cnt = 1u;
    }

    return true;
}

struct EvaluateNodeContext
{
    std::uint32_t elements = 0u;
    const char* p_current = nullptr;
};
bool evaluate_node(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind, const Lexeme& lexeme_a, void* p_user_data_a)
{
    auto* p_context = reinterpret_cast<EvaluateNodeContext*>(p_user_data_a);

    if (1u == context_size_a && (1u == current_transition_a || 2u == current_transition_a || current_transition_a == 10u || current_transition_a == 17u))
    {
        p_context->elements++;
    }

    if (1u == context_size_a && (lexeme_a.value == "}" || lexeme_a.value == "]"))
    {
        p_context->p_current = lexeme_a.value.data() + 1u;
        return false;
    }

    return true;
}

struct EvaluateArrayElementContext
{
    std::size_t index = 0u;
    std::size_t curr_cnt = 0;
    const char* p_current = nullptr;
};
bool evaluate_array_element_value(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind, const Lexeme& lexeme_a, void* p_user_data_a)
{
    auto* p_context = reinterpret_cast<EvaluateArrayElementContext*>(p_user_data_a);

    if ((current_transition_a >= 11u && current_transition_a <= 15u && 1u == context_size_a) ||
        ((0u == current_transition_a || 16u == current_transition_a) && 2u == context_size_a))
    {
        if (p_context->index == p_context->curr_cnt && (current_transition_a >= 11u && current_transition_a <= 15u && 1u == context_size_a))
        {
            p_context->p_current = Lexeme::string == lexeme_a.kind ? lexeme_a.value.data() - 1u : lexeme_a.value.data();
            return false;
        }

        p_context->curr_cnt++;
    }

    return true;
}
bool evaluate_array_element_object(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind, const Lexeme& lexeme_a, void* p_user_data_a)
{
    auto* p_context = reinterpret_cast<EvaluateArrayElementContext*>(p_user_data_a);

    if ((current_transition_a >= 11u && current_transition_a <= 15u && 1u == context_size_a) ||
        ((0u == current_transition_a || 16u == current_transition_a) && 2u == context_size_a))
    {
        if (p_context->index == p_context->curr_cnt && (0u == current_transition_a && 2u == context_size_a))
        {
            p_context->p_current = lexeme_a.value.data();
            return false;
        }

        p_context->curr_cnt++;
    }

    return true;
}
bool evaluate_array_element_array(std::size_t current_transition_a, std::size_t context_size_a, Scope::Kind, const Lexeme& lexeme_a, void* p_user_data_a)
{
    auto* p_context = reinterpret_cast<EvaluateArrayElementContext*>(p_user_data_a);

    if ((current_transition_a >= 11u && current_transition_a <= 15u && 1u == context_size_a) ||
        ((0u == current_transition_a || 16u == current_transition_a) && 2u == context_size_a))
    {
        if (p_context->index == p_context->curr_cnt && (16u == current_transition_a && 2u == context_size_a))
        {
            p_context->p_current = lexeme_a.value.data();
            return false;
        }

        p_context->curr_cnt++;
    }

    return true;
}
} // namespace

namespace xjson {
Document::Document(std::string_view document_data_a)
    : data(document_data_a)
    , valid(evaluate_json(this->data))
{
}

template<> Document::Value Document::get_root() const
{
    const auto lexeme = evaluate_next_lexeme(this->data);

    if (Lexeme::string == lexeme.kind || Lexeme::number == lexeme.kind || Lexeme::keyword == lexeme.kind)
    {
        return lexeme.value;
    }

    return {};
}
template<> Document::Object Document::get_root() const
{
    const auto lexeme = evaluate_next_lexeme(this->data);

    if (Lexeme::separator == lexeme.kind && "{" == lexeme.value)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                this->data.data(),
                this->data.data() + this->data.size(),
            },
            { .function = evaluate_node, .p_user_data = &evaluate_node_context });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, this->data.data(), evaluate_node_context.p_current };
        }
    }

    return {};
}
template<> Document::Array Document::get_root() const
{
    const auto lexeme = evaluate_next_lexeme(this->data);

    if (Lexeme::separator == lexeme.kind && "[" == lexeme.value)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                this->data.data(),
                this->data.data() + this->data.size(),
            },
            { .function = evaluate_node, .p_user_data = &evaluate_node_context });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, this->data.data(), evaluate_node_context.p_current };
        }
    }

    return {};
}

template<> Document::Value Document::Object::get(std::string_view key_a) const
{
    FindKeyContext context { .key = key_a };
    bool find_key_res = evaluate_json({ this->p_begin, this->p_end }, { .function = find_key, .p_user_data = &context });

    if (true == find_key_res && nullptr != context.p_current)
    {
        const auto lexeme = evaluate_next_lexeme({ context.p_current, this->p_end });
        return lexeme.value;
    }

    return {};
}
template<> Document::Object Document::Object::get(std::string_view key_a) const
{
    FindKeyContext find_key_context { .key = key_a };
    bool find_key_res = evaluate_json({ this->p_begin, this->p_end }, { .function = find_key, .p_user_data = &find_key_context });

    if (true == find_key_res && nullptr != find_key_context.p_current)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                find_key_context.p_current,
                this->p_end,
            },
            { .function = evaluate_node, .p_user_data = &evaluate_node_context });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, find_key_context.p_current, evaluate_node_context.p_current };
        }
    }

    return {};
}
template<> Document::Array Document::Object::get(std::string_view key_a) const
{
    FindKeyContext find_key_context { .key = key_a };
    bool find_key_res = evaluate_json({ this->p_begin, this->p_end }, { .function = find_key, .p_user_data = &find_key_context });

    if (true == find_key_res && nullptr != find_key_context.p_current)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                find_key_context.p_current,
                this->p_end,
            },
            { .function = evaluate_node, .p_user_data = reinterpret_cast<void*>(&evaluate_node_context) });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, find_key_context.p_current, evaluate_node_context.p_current };
        }
    }

    return {};
}

template<> Document::Value Document::Array::get(std::size_t index_a) const
{
    assert(index_a < this->elements_count);

    EvaluateArrayElementContext evaluate_array_element_context { .index = index_a };
    bool evaluate_node_res =
        evaluate_json({ this->p_begin, this->p_end }, { .function = evaluate_array_element_value, .p_user_data = &evaluate_array_element_context });

    if (true == evaluate_node_res && nullptr != evaluate_array_element_context.p_current)
    {
        const auto lexeme = evaluate_next_lexeme({ evaluate_array_element_context.p_current, this->p_end });
        return lexeme.value;
    }

    return {};
}
template<> Document::Object Document::Array::get(std::size_t index_a) const
{
    assert(index_a < this->elements_count);

    EvaluateArrayElementContext evaluate_array_element_context { .index = index_a };
    bool evaluate_node_res =
        evaluate_json({ this->p_begin, this->p_end }, { .function = evaluate_array_element_object, .p_user_data = &evaluate_array_element_context });

    if (true == evaluate_node_res && nullptr != evaluate_array_element_context.p_current)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                evaluate_array_element_context.p_current,
                this->p_end,
            },
            { .function = evaluate_node, .p_user_data = reinterpret_cast<void*>(&evaluate_node_context) });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, evaluate_array_element_context.p_current, evaluate_node_context.p_current };
        }
    }

    return {};
}
template<> Document::Array Document::Array::get(std::size_t index_a) const
{
    assert(index_a < this->elements_count);

    EvaluateArrayElementContext evaluate_array_element_context { .index = index_a };
    bool evaluate_node_res =
        evaluate_json({ this->p_begin, this->p_end }, { .function = evaluate_array_element_array, .p_user_data = &evaluate_array_element_context });

    if (true == evaluate_node_res && nullptr != evaluate_array_element_context.p_current)
    {
        EvaluateNodeContext evaluate_node_context;
        bool evaluate_node_res = evaluate_json(
            {
                evaluate_array_element_context.p_current,
                this->p_end,
            },
            { .function = evaluate_node, .p_user_data = reinterpret_cast<void*>(&evaluate_node_context) });

        if (true == evaluate_node_res && nullptr != evaluate_node_context.p_current)
        {
            return { evaluate_node_context.elements, evaluate_array_element_context.p_current, evaluate_node_context.p_current };
        }
    }

    return {};
}
} // namespace xjson
