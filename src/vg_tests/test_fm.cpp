//
// Created by III on 2025/7/27.
//
#include <catch2/catch_test_macros.hpp>
#include "MockFMResultSet.h"
#include <stdexcept>
#include <cmath>

TEST_CASE("FMResultSet interface comprehensive test") {
    MockFMResultSet rs;

    SECTION("next() behavior") {
        REQUIRE(rs.next());
        REQUIRE_FALSE(rs.next());
    }

    SECTION("columnCount and columnName") {
        REQUIRE(rs.columnCount() == 3);
        REQUIRE(rs.columnName(0) == "id");
        REQUIRE(rs.columnName(1) == "name");
        REQUIRE(rs.columnName(2) == "null");
        REQUIRE(rs.columnName(3).empty());
    }

    SECTION("getInt/getDouble/getString by column name") {
        auto intRes = rs.getInt("id");
        REQUIRE(intRes.isOk());
        REQUIRE(intRes.value().has_value());
        REQUIRE(intRes.value().value() == 123);

        auto dblRes = rs.getDouble("name");
        REQUIRE(dblRes.isOk());
        REQUIRE(dblRes.value().has_value());
        REQUIRE(std::abs(dblRes.value().value() - 45.6) < 1e-6);

        auto strRes = rs.getString("name");
        REQUIRE(strRes.isOk());
        REQUIRE(strRes.value().has_value());
        REQUIRE(strRes.value().value() == "test");

        auto errRes = rs.getInt("error");
        REQUIRE(errRes.isErr());
        REQUIRE_THROWS_AS(errRes.value(), std::logic_error);
    }

    SECTION("getInt/getDouble/getString by column index") {
        auto intRes = rs.getInt(1);
        REQUIRE(intRes.isOk());
        REQUIRE(intRes.value().has_value());
        REQUIRE(intRes.value().value() == 789);

        auto dblRes = rs.getDouble(0);
        REQUIRE(dblRes.isOk());
        REQUIRE(dblRes.value().has_value());
        REQUIRE(std::abs(dblRes.value().value() - 12.34) < 1e-6);
        auto strRes = rs.getString(2);
        REQUIRE(strRes.isOk());
        REQUIRE(strRes.value().has_value());
        REQUIRE(strRes.value().value() == "mock");

        auto errRes = rs.getString(5);
        REQUIRE(errRes.isErr());
        REQUIRE_THROWS_AS(errRes.value(), std::logic_error);
    }

    SECTION("isNull by column name") {
        auto nullRes = rs.isNull("null");
        REQUIRE(nullRes.isOk());
        REQUIRE(nullRes.value() == true);

        auto notNullRes = rs.isNull("id");
        REQUIRE(notNullRes.isOk());
        REQUIRE(notNullRes.value() == false);

        auto errRes = rs.isNull("error");
        REQUIRE(errRes.isErr());
        REQUIRE_THROWS_AS(errRes.value(), std::logic_error);
    }

    SECTION("isNull by column index") {
        auto nullRes = rs.isNull(2);
        REQUIRE(nullRes.isOk());
        REQUIRE(nullRes.value() == true);

        auto notNullRes = rs.isNull(1);
        REQUIRE(notNullRes.isOk());
        REQUIRE(notNullRes.value() == false);

        auto errRes = rs.isNull(10);
        REQUIRE(errRes.isErr());
        REQUIRE_THROWS_AS(errRes.value(), std::logic_error);
    }
}