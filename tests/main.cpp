/*
 *  Copyright (c) xEmbeddedTools team and contributors.
 *  Licensed under the Apache License, Version 2.0. See LICENSE file in the project root for details.
 */

// externals
#include <catch2/catch.hpp>

// xjson
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
    xjson::Document document("1");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("negative integer isvalid")
{
    xjson::Document document("-1");
    REQUIRE(true == document.is_valid());
}
TEST_CASE("positive floating point is valid")
{
    xjson::Document document("1.0");
    REQUIRE(true == document.is_valid());
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
    {
        constexpr std::string_view json = R"({"field":{}})";
        xjson::Document document(json);
        REQUIRE(true == document.is_valid());
    }
}

TEST_CASE("incorrect number")
{
    {
        xjson::Document document("0.avc");
        REQUIRE(false == document.is_valid());
    }

    {
        xjson::Document document("00.1234");
        REQUIRE(false == document.is_valid());
    }

    {
        xjson::Document document("01");
        REQUIRE(false == document.is_valid());
    }
    {
        xjson::Document document("-01");
        REQUIRE(false == document.is_valid());
    }
    {
        xjson::Document document(".1");
        REQUIRE(false == document.is_valid());
    }
    {
        xjson::Document document("1.");
        REQUIRE(false == document.is_valid());
    }
    {
        xjson::Document document("1.0.0");
        REQUIRE(false == document.is_valid());
    }
    {
        xjson::Document document("-");
        REQUIRE(false == document.is_valid());
    }
}

int main(int argc, char* argv[])
{
    Catch::Session().run(argc, argv);
}
