#include <catch2/catch.hpp>
#include <xjson/Document.hpp>

TEST_CASE("empty array is valid")
{
    xjson::Document document("[]");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("empty object is valid")
{
    xjson::Document document("{}");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("empty object nested in array is valid")
{
    xjson::Document document("[{}]");
    REQUIRE(true == document.is_valid());
}

TEST_CASE("string is valid")
{
    xjson::Document document("\"test\"");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("positive integer is valid")
{
    {
        xjson::Document document("1");
        REQUIRE(true == document.is_valid());
    }
    {
        xjson::Document document("+1");
        REQUIRE(true == document.is_valid());
    }
}
TEST_CASE("negative integer isvalid")
{
    xjson::Document document("-1");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("positive floating point is valid")
{
    {
        xjson::Document document("1.0");
        REQUIRE(true == document.is_valid());
    }
    {
        xjson::Document document("+1.0");
        REQUIRE(true == document.is_valid());
    }
}
TEST_CASE("negative floating point is valid")
{
    xjson::Document document("-1.0");
    REQUIRE(true == document.is_valid());
}

TEST_CASE("keyword is valid")
{
    {
        xjson::Document document("true");
        REQUIRE(true == document.is_valid());
    }
    {
        xjson::Document document("false");
        REQUIRE(true == document.is_valid());
    }
    {
        xjson::Document document("null");
        REQUIRE(true == document.is_valid());
    }
}

TEST_CASE("simple object")
{
    {
        constexpr std::string_view json = R"({"key":"value"})";

        xjson::Document document(json);
        REQUIRE(true == document.is_valid());
    }
    {
        constexpr std::string_view json =
            R"(
{"key":"value"}
)";

        xjson::Document document(json);
        REQUIRE(true == document.is_valid());
    }
    {
        constexpr std::string_view json =
            R"(
     {"key":"value"}
)";

        xjson::Document document(json);
        REQUIRE(true == document.is_valid());
    }
}

int main(int argc, char* argv[])
{
    Catch::Session().run(argc, argv);
}