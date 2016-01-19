/*
 * Copyright (c) 2016 Andreas Pohl
 * Licensed under MIT (see COPYING)
 *
 * Author: Andreas Pohl
 */

#include "lua_engine.h"
#include "builtin/hash.h"

#include <boost/utility/string_ref.hpp>

#define BOOST_TEST_MODULE test_hash
//#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace petrel;
using namespace petrel::lib;

BOOST_AUTO_TEST_SUITE(test_hash_suite);

BOOST_AUTO_TEST_CASE(test_md5) {
    lua_engine e;
    auto Lex = e.create_lua_state();
    // create test function
    const std::string lua_code = "function test_md5(str) return hash.md5(str) end";
    luaL_dostring(Lex.L, lua_code.c_str());
    // load test function
    lua_getglobal(Lex.L, "test_md5");
    BOOST_CHECK(lua_isfunction(Lex.L, -1));
    // push function parameter
    lua_pushstring(Lex.L, "petrel roxx :-)");
    // call function
    BOOST_CHECK(!lua_pcall(Lex.L, 1, 1, Lex.traceback_idx));
    // check result
    BOOST_CHECK(lua_isstring(Lex.L, -1));
    boost::string_ref ret(lua_tostring(Lex.L, -1));
    BOOST_CHECK(ret == "d482fa70a78ca1fffbfff6e0d0d314a5");
    e.destroy_lua_state(Lex);
}

BOOST_AUTO_TEST_CASE(test_sha1) {
    lua_engine e;
    auto Lex = e.create_lua_state();
    // create test function
    const std::string lua_code = "function test_sha1(str) return hash.sha1(str) end";
    luaL_dostring(Lex.L, lua_code.c_str());
    // load test function
    lua_getglobal(Lex.L, "test_sha1");
    BOOST_CHECK(lua_isfunction(Lex.L, -1));
    // push function parameter
    lua_pushstring(Lex.L, "petrel is super :-)");
    // call function
    BOOST_CHECK(!lua_pcall(Lex.L, 1, 1, Lex.traceback_idx));
    // check result
    BOOST_CHECK(lua_isstring(Lex.L, -1));
    boost::string_ref ret(lua_tostring(Lex.L, -1));
    BOOST_CHECK(ret == "c6b64747e8b1302220982c33b541981838fd28dd");
    e.destroy_lua_state(Lex);
}

BOOST_AUTO_TEST_CASE(test_fnv) {
    lua_engine e;
    auto Lex = e.create_lua_state();
    // create test function
    const std::string lua_code = "function test_fnv(str) return hash.fnv(str) end";
    luaL_dostring(Lex.L, lua_code.c_str());
    // load test function
    lua_getglobal(Lex.L, "test_fnv");
    BOOST_CHECK(lua_isfunction(Lex.L, -1));
    // push function parameter
    lua_pushstring(Lex.L, "petrel yeah :-)");
    // call function
    BOOST_CHECK(!lua_pcall(Lex.L, 1, 1, Lex.traceback_idx));
    // check result
    BOOST_CHECK(lua_isinteger(Lex.L, -1));
    auto ret = static_cast<std::uint32_t>(lua_tointeger(Lex.L, -1));
    BOOST_CHECK(ret == 1012600862);
    e.destroy_lua_state(Lex);
}

BOOST_AUTO_TEST_SUITE_END();
