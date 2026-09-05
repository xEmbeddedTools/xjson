// This file is part of the xmcu project.
// Licensed under the Apache License, Version 2.0 (the \"License\")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an \"AS IS\" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// externals
#include <catch2/catch.hpp>

// xjson
#include <xjson/Document.hpp>

TEST_CASE("scalar values", "[validation]")
{
    SECTION("strings")
    {
        REQUIRE(true == xjson::Document(R"("text")").is_valid());
        REQUIRE(true == xjson::Document(R"("")").is_valid());
    }
    SECTION("integers and decimals")
    {
        for (const auto json : { "0", "42", "-42", "0.25", "-0.25" })
        {
            REQUIRE(true == xjson::Document(json).is_valid());
        }
    }
    SECTION("keywords")
    {
        for (const auto json : { "true", "false", "null" })
        {
            REQUIRE(true == xjson::Document(json).is_valid());
        }
    }
}

TEST_CASE("accepts supported compound JSON values and whitespace", "[validation]")
{
    REQUIRE(true == xjson::Document("{}").is_valid());
    REQUIRE(true == xjson::Document("[]").is_valid());
    REQUIRE(true == xjson::Document(R"( { "name" : "Ada", "scores" : [ 10, 20 ] } )").is_valid());
    REQUIRE(true == xjson::Document(R"(
[
  { "enabled": true },
  null
]
)")
                        .is_valid());
}

TEST_CASE("rejects malformed JSON", "[validation]")
{
    for (const auto json : { "", " ", "\n", "{", "[", "{\"key\"}", "{\"key\":}", "[1,]", "{\"key\":1,}", "[1 2]", "{\"key\":1 \"next\":2}" })
    {
        REQUIRE(false == xjson::Document(json).is_valid());
    }
}

TEST_CASE("rejects unsupported number spellings", "[validation]")
{
    for (const auto json : { "00", "01", "-01", ".1", "1.", "1.0.0", "-", "1e3", "0.avc" })
    {
        REQUIRE(false == xjson::Document(json).is_valid());
    }
}

TEST_CASE("reads scalar roots", "[root]")
{
    {
        xjson::Document document(R"("Ada")");
        const auto root = document.get_root<xjson::Document::Value>();
        REQUIRE("Ada" == root);
    }
    {
        xjson::Document document(R"("-12.5")");
        const auto root = document.get_root<xjson::Document::Value>();
        REQUIRE("-12.5" == root);
    }
    {
        xjson::Document document(R"("true")");
        const auto root = document.get_root<xjson::Document::Value>();
        REQUIRE("true" == root);
    }
    {
        xjson::Document document(R"("null")");
        const auto root = document.get_root<xjson::Document::Value>();
        REQUIRE("null" == root);
    }
}

TEST_CASE("reads object roots and their scalar fields", "[object][root]")
{
    constexpr std::string_view json = R"({"name":"Ada","age":37,"active":true,"middle":null})";
    const auto document = xjson::Document(json);
    REQUIRE(true == document.is_valid());

    const auto object = document.get_root<xjson::Document::Object>();
    REQUIRE(true == object);

    REQUIRE(4u == object.fields_count);
    REQUIRE("Ada" == object.get<xjson::Document::Value>("name"));
    REQUIRE("37" == object.get<xjson::Document::Value>("age"));
    REQUIRE("true" == object.get<xjson::Document::Value>("active"));
    REQUIRE("null" == object.get<xjson::Document::Value>("middle"));
    REQUIRE("" == object.get<xjson::Document::Value>("absent"));
}

TEST_CASE("reads nested objects and arrays from an object", "[object][nested]")
{
    constexpr std::string_view json = R"({"profile":{"name":"Ada","role":"admin"},"scores":[10,20,30]})";
    const auto document = xjson::Document(json);
    REQUIRE(true == document.is_valid());

    const auto object = document.get_root<xjson::Document::Object>();
    REQUIRE(true == object);

    const auto profile = object.get<xjson::Document::Object>("profile");
    REQUIRE(true == profile);

    REQUIRE(2u == profile.fields_count);
    REQUIRE("Ada" == profile.get<xjson::Document::Value>("name"));
    REQUIRE("admin" == profile.get<xjson::Document::Value>("role"));

    const auto scores = object.get<xjson::Document::Array>("scores");
    REQUIRE(true == scores);

    REQUIRE(3u == scores.elements_count);
    REQUIRE("10" == scores.get<xjson::Document::Value>(0u));
    REQUIRE("20" == scores.get<xjson::Document::Value>(1u));
    REQUIRE("30" == scores.get<xjson::Document::Value>(2u));

    REQUIRE(false == object.get<xjson::Document::Object>("missing"));
    REQUIRE(false == object.get<xjson::Document::Array>("profile"));
}

TEST_CASE("reads heterogeneous arrays and nested nodes by index", "[array][nested]")
{
    constexpr std::string_view json = R"(["text",42,false,null,{"id":7},["nested",2]])";
    const auto document = xjson::Document(json);
    REQUIRE(true == document.is_valid());

    const auto array = document.get_root<xjson::Document::Array>();
    REQUIRE(true == array);

    REQUIRE(true == array);
    REQUIRE(6u == array.elements_count);
    REQUIRE("text" == array.get<xjson::Document::Value>(0u));
    REQUIRE("42" == array.get<xjson::Document::Value>(1u));
    REQUIRE("false" == array.get<xjson::Document::Value>(2u));
    REQUIRE("null" == array.get<xjson::Document::Value>(3u));

    const auto object = array.get<xjson::Document::Object>(4u);
    REQUIRE(true == object);
    REQUIRE(1u == object.fields_count);
    REQUIRE("7" == object.get<xjson::Document::Value>("id"));

    const auto nested = array.get<xjson::Document::Array>(5u);
    REQUIRE(true == nested);
    REQUIRE(2u == nested.elements_count);
    REQUIRE("nested" == nested.get<xjson::Document::Value>(0u));
    REQUIRE("2" == nested.get<xjson::Document::Value>(1u));

    const auto invalid_array = array.get<xjson::Document::Array>(4u);
    REQUIRE(false == invalid_array);

    const auto invalid_object = array.get<xjson::Document::Object>(5u);
    REQUIRE(false == invalid_object);
}

TEST_CASE("returns an empty node when the requested root type does not match", "[root]")
{
    const auto scalar_document = xjson::Document("42");
    REQUIRE(true == scalar_document.is_valid());

    REQUIRE(false == scalar_document.get_root<xjson::Document::Object>());
    REQUIRE(false == scalar_document.get_root<xjson::Document::Array>());

    const auto object_document = xjson::Document("{}");
    REQUIRE(true == object_document.is_valid());

    REQUIRE(false == object_document.get_root<xjson::Document::Array>());
}

TEST_CASE("reports zero fields and elements for empty compound nodes", "[object][array]")
{
    const auto object_document = xjson::Document("{}");
    const auto array_document = xjson::Document("[]");

    REQUIRE(true == object_document.is_valid());
    REQUIRE(true == array_document.is_valid());

    const auto object = object_document.get_root<xjson::Document::Object>();
    const auto array = array_document.get_root<xjson::Document::Array>();

    REQUIRE(true == object);
    REQUIRE(true == array);
    REQUIRE(0u == object.fields_count);
    REQUIRE(0u == array.elements_count);
}

int main(int argc, char* argv[])
{
    Catch::Session().run(argc, argv);
}
