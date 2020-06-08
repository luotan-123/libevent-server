#pragma once

#include "lua.hpp"

extern "C"
{
int luaopen_pb(lua_State* L);
int luaopen_luasql_mysql(lua_State* L);
}