# xjson

`xjson` is a small C++20 library for validating and reading JSON data. It exposes document data through `std::string_view`, so retrieving values does not require copying them.

## Requirements

- A compiler with C++20 support
- CMake 3.21 or newer

## Adding it to a CMake project

Add the project as a subdirectory, then link your program with the `xjson` target:

```cmake
add_subdirectory(path/to/xjson)
target_link_libraries(my_application PRIVATE xjson)
```

The public API header is available at:

```cpp
#include <xjson/Document.hpp>
```

## Example

```cpp
#include <xjson/Document.hpp>
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

A missing object or array converts to `false`. Scalar values are returned as `Document::Value`, which is a `std::string_view`; for a JSON string, it does not include the surrounding quotation marks.

> `Document` does not own its input data. The text passed to its constructor must remain alive while the document, and any objects or arrays obtained from it, are in use.

## Tests

The tests use Catch2, whose source is included in `tests/externals/catch2`.

```powershell
cmake -S tests -B tests/out/build
cmake --build tests/out/build --config Debug
```

Then run the generated `tests` executable (`tests.exe` on Windows) from the output directory appropriate for your chosen generator and build configuration.