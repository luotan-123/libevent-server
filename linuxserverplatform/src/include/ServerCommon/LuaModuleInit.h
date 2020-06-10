#pragma once

#include "lua.hpp"
#include "luasocket.h"

extern "C"
{
int luaopen_pb(lua_State* L);
int luaopen_luasql_mysql(lua_State* L);
}