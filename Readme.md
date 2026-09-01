# xjson

`xjson` is a small C++20 library for validating and reading JSON data. It performs no dynamic memory allocation, making it suitable for memory-constrained embedded systems.

### Requirements

- A compiler with C++20 support
- CMake 3.12 or newer

# Quick start

### CMake project integration

Add the project as a subdirectory, then link your program with the `xjson` target:

```cmake
add_subdirectory(path/to/xjson)
target_link_libraries(my_application PRIVATE xjson)
```

## Example

```cpp

// xjson
#include <xjson/Document.hpp>

// std
#include <iostream>
#include <string_view>

int main()
{
    constexpr std::string_view json = R"({"name":"Ada","scores":[10,20]})";

    xjson::Document document(json);
    if (!document.is_valid())
    {
        return 1;
    }

    const auto root = document.get_root<xjson::Document::Object>();
    const auto name = root.get<xjson::Document::Value>("name");
    const auto scores = root.get<xjson::Document::Array>("scores");

    std::cout << name << ": " << scores.get<xjson::Document::Value>(0) << '\n';
}
```

Output:

```text
Ada: 10
```

## API

`xjson::Document` accepts JSON text in its constructor.

- `is_valid()` checks whether the document was parsed successfully.
- `get_root<T>()` returns the root as `Document::Value`, `Document::Object`, or `Document::Array`.
- `Object::get<T>(key)` retrieves a field value.
- `Array::get<T>(index)` retrieves an array element; the index must be less than `elements_count`.
- `Object::fields_count` and `Array::elements_count` contain the number of fields and elements, respectively.

A missing Value, Object or Array converts to `false`.

> `Document` does not own its input data. The text passed to its constructor must remain alive while the document, and any objects or arrays obtained from it, are in use.

## Intended use

`xjson` is intended primarily for embedded systems where predictable memory usage and avoiding heap allocation are important. It works directly on the received JSON text and does not build a separate in-memory representation of the document or a separate token list.

### Configuring nesting depth

`XJSON_MAX_NESTING_DEPTH` configures the maximum object and array nesting depth at compile time. Its default value is `17`.

Set the macro when compiling the `xjson` target:

```cmake
add_subdirectory(path/to/xjson)
target_compile_definitions(xjson PRIVATE XJSON_MAX_NESTING_DEPTH=8)
```

## Current limitations

`xjson` is not yet fully compliant with [RFC 8259](https://www.rfc-editor.org/info/rfc8259/). In particular:

- Exponent notation such as `1e3` is not supported. Numeric lexemes of 32 or more characters are rejected.
- JSON string escape sequences, including `\"`, `\\`, `\n`, and `\uXXXX`, are not supported.
- Unescaped control characters inside strings may be accepted.
- Nesting depth is limited by `XJSON_MAX_NESTING_DEPTH` (17 by default); more deeply nested documents are rejected.

Do not use `is_valid()` as proof of strict RFC 8259 conformance. Validate the expected input subset thoroughly before using the library in production.

## Tests

The tests use Catch2, whose source is included in `tests/externals/catch2`.

```powershell
cmake -S tests -B tests/out/build
cmake --build tests/out/build --config Debug
```

Then run the generated `tests` executable (`tests.exe` on Windows) from the output directory appropriate for your chosen generator and build configuration.
