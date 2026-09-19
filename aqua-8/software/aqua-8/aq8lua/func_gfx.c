#define LUA_LIB
#include "func_gfx.h"

int laq8_print(lua_State *L) {
    // Get number of arguments
    int n = lua_gettop(L);

    lua_getglobal(L, "tostring");

    for (int i = 1; i <= n; i++) {
        const char *s;
        size_t      l;
        lua_pushvalue(L, -1); // function to be called
        lua_pushvalue(L, i);  // value to print
        lua_call(L, 1, 1);

        s = lua_tolstring(L, -1, &l); // get result
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        if (i > 1)
            luai_writestring("\t", 1);

        luai_writestring(s, l);
        lua_pop(L, 1); // pop result
    }

    luai_writeline();
    return 0;
}

int laq8_cls(lua_State *L) {
    int n = lua_gettop(L); /* number of arguments */

    int color_idx = 0;
    if (n > 0) {
        if (!lua_isnumber(L, 1)) {
            lua_pushstring(L, "incorrect argument");
            lua_error(L);
        }
        color_idx = (lua_tonumber(L, 1) >> 16) & 15;
    }

    printf("cls(%d)\n", color_idx);
    // printf("cls! n=%d\n", n);

    // // lua_pushnumber(L, 123 << 16);
    // lua_pushstring(L, "dinges");
    // lua_pushinteger(L, 123 << 16);
    return 0;
}
