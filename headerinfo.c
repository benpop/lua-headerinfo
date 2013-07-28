/*
** Place Lua function dump header information into a table
**
** Author: benpop
** Date: 2013-07-27
*/


#include <string.h>

#include "lua.h"
#include "lauxlib.h"


#ifndef LUA_OK
#define LUA_OK 0
#endif


#define SIGSIZE       (sizeof(LUA_SIGNATURE)-1)

#define INFOSIZE      8

/* size in bytes of header of binary files or string dumps,
   at least the part we care about */
#define HEADERSIZE    (SIGSIZE + INFOSIZE)

/* header info byte offset */
#define GetInfo(s,i)  ((unsigned char)(s[SIGSIZE + (i)]))


static int Error (lua_State *L, const char *msg) {
  lua_pushnil(L);
  lua_pushstring(L, msg);
  return 2;
}


static int Writer (lua_State *L, const void *s, size_t l, void *B) {
  (void)L;
  luaL_addlstring((luaL_Buffer *)B, (const char *)s, l);
  return LUA_OK;
}


static int headerinfo (lua_State *L) {
  const char *dump;
  lua_settop(L, 1);
  if (lua_isnil(L, 1)) {
    if (luaL_loadstring(L, "return function () end") != LUA_OK)
      return Error(L, "unable to load dummy function");
    lua_replace(L, 1);
  }
  switch (lua_type(L, 1)) {
    case LUA_TSTRING: {
      size_t l;
      dump = lua_tolstring(L, 1, &l);
      if (l < HEADERSIZE || strncmp(LUA_SIGNATURE, dump, SIGSIZE) != 0)
        return Error(L, "not a valid Lua function dump");
      break;
    }
    case LUA_TFUNCTION: {
      luaL_Buffer b;
      luaL_buffinit(L, &b);
      if (lua_dump(L, Writer, &b) != LUA_OK)
        return Error(L, "unable to dump given function");
      luaL_pushresult(&b);
      lua_replace(L, 1);
      dump = lua_tostring(L, 1);
      break;
    }
    default: {
      return Error(L, "not a function or function dump");
    }
  }

  lua_createtable(L, 0, INFOSIZE);

  {
    int major = (GetInfo(dump, 0) >> 4);
    int minor = (GetInfo(dump, 0) & 0xf);
    lua_pushfstring(L, "%d.%d", major, minor);
    lua_setfield(L, 2, "version");
  }

  lua_pushinteger(L, GetInfo(dump, 1));
  lua_setfield(L, 2, "format");

  lua_pushstring(L, (GetInfo(dump, 2) ? "little-endian" : "big-endian"));
  lua_setfield(L, 2, "endianness");

  lua_pushinteger(L, GetInfo(dump, 3));
  lua_setfield(L, 2, "int_size");

  lua_pushinteger(L, GetInfo(dump, 4));
  lua_setfield(L, 2, "size_size");

  lua_pushinteger(L, GetInfo(dump, 5));
  lua_setfield(L, 2, "instruction_size");

  lua_pushinteger(L, GetInfo(dump, 6));
  lua_setfield(L, 2, "number_size");

  lua_pushboolean(L, GetInfo(dump, 7));
  lua_setfield(L, 2, "number_is_int");

  return 1;
}


int luaopen_headerinfo (lua_State *L) {
  lua_pushcfunction(L, headerinfo);
  lua_pushvalue(L, -1);
  lua_setglobal(L, "headerinfo");
  return 1;
}
