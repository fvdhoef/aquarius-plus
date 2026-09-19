#include "aq8lua.h"

#define LUA_LIB
#include "z8lua/lua.h"
#include "z8lua/lauxlib.h"
#include "z8lua/lualib.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include "func_gfx.h"

extern "C" {
#include "state.h"
#include "draw/draw.h"
}

static void mylib_register(lua_State *L);

static void print_lua_error(const char *type, lua_State *L) {
    printf("\fE%s\f6\n", type);

    if (!lua_isnil(L, -1)) {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL)
            msg = "(error object is not a string)";

        if (memcmp(msg, "[string \"C\"]:", 13) == 0) {
            msg += 13;
            printf("Line %s\n", msg);
        } else if (memcmp(msg, "[string \"I\"]:0: ", 16) == 0) {
            msg += 16;
            if (strncmp(msg, "syntax error", 12) != 0)
                printf("%s\n", msg);
        } else {
            printf("%s\n", msg);
        }

        lua_pop(L, 1);

        // force a complete garbage collection in case of errors
        lua_gc(L, LUA_GCCOLLECT, 0);
    }
}

void aq8lua_shutdown(void) {
    if (game_state.lua_state) {
        lua_close((lua_State *)game_state.lua_state);
        game_state.lua_state = NULL;
    }
}

void aq8lua_init(void) {
    if (game_state.lua_state) {
        return;
    }

    lua_State *L = luaL_newstate();
    if (L == NULL) {
        printf("Error creating Lua state\n");
        return;
    }
    game_state.lua_state = L;

    luaL_openlibs(L);
    mylib_register(L);
}

static bool _lua_fn_exists(const char *fn) {
    lua_State *L = (lua_State *)game_state.lua_state;
    lua_getglobal(L, fn);
    if (lua_isfunction(L, -1)) {
        return true;
    } else {
        // printf("Function %s does not exist\n", fn);
        return false;
    }
}

static uint8_t _to_lua_call(const char *fn) {
    lua_State *L = (lua_State *)game_state.lua_state;
    lua_getglobal(L, fn);
    if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
        return 0;
    } else {
        printf("Lua error: %s\n", lua_tostring(L, lua_gettop(L)));
        lua_pop(L, lua_gettop(L));
        return 1;
    }
}

bool aq8lua_run(const char *name, const void *buf, unsigned size) {
    if (!game_state.lua_state) {
        return false;
    }

    if (size >= 256) {
        printf("Parsing code...\n");
    }

    lua_State *L = (lua_State *)game_state.lua_state;

    int result = luaL_loadbufferx(L, (const char *)buf, size, name, "text");
    if (result != LUA_OK) {
        print_lua_error("Syntax error", L);
        return false;
    }

    if (size >= 256) {
        printf("Running...\n");
    }

    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        print_lua_error("Runtime error", L);
        return false;
    }
    return true;
}

void aq8lua_gameloop(void) {
    if (_lua_fn_exists("_init"))
        if (_to_lua_call("_init")) {
            return;
        }

    bool call_update = _lua_fn_exists("_update");
    bool call_draw   = _lua_fn_exists("_draw");

    if (call_draw) {
        unsigned page = 2;
        VIDEO->PAGE   = page;

        while (1) {
            VIDEO->PALETTE[0] = 0x600;
            if (call_update)
                if (_to_lua_call("_update"))
                    return;
            if (call_draw)
                if (_to_lua_call("_draw"))
                    return;

            palette_init();

            frame60 = false;
            while (!frame60) {
            }
            page ^= 3;
            VIDEO->PAGE = page;
        }
    }
}

static int aq8_cls(lua_State *L) {
    uint8_t palIdx = luaL_optinteger(L, 1, 0);
    clear_screen(palIdx & 0xF);
    return 0;
}

static int aq8_time(lua_State *L) {
    float t = (float)frame_cnt / 60.0f;

    lua_pushnumber(L, t);
    return 1;
}

static int aq8_pset(lua_State *L) {
    int16_t x   = luaL_checkinteger(L, 1);
    int16_t y   = luaL_checkinteger(L, 2);
    uint8_t idx = luaL_optinteger(L, 3, game_state.color);

    draw_pixel(x, y, idx);

    // printf("pset(%d,%d,%d)\n", x, y, idx);
    return 0;
}

static int aq8_pal(lua_State *L) {
    return 0;
}
static int aq8_palt(lua_State *L) {
    return 0;
}
static int aq8_spr(lua_State *L) {
    uint8_t argcount = lua_gettop(L);
    if (argcount < 3)
        return 0;

    int n = luaL_optinteger(L, 1, -1);
    if (n == -1)
        return 0;
    int       x = luaL_checkinteger(L, 2);
    int       y = luaL_checkinteger(L, 3);
    z8::fix32 w = luaL_optinteger(L, 4, 1.0);
    z8::fix32 h = luaL_optinteger(L, 5, 1.0);

    bool flip_x = false;
    bool flip_y = false;

    if (argcount >= 6)
        flip_x = lua_toboolean(L, 6);
    if (argcount >= 7)
        flip_y = lua_toboolean(L, 7);

    draw_game_sprite(n, x, y);
    // spr(n, x, y, w, h, flip_x, flip_y);

    return 0;
}

static int aq8_print(lua_State *L) {
    size_t      textLen    = 0;
    const char *text       = luaL_checklstring(L, 1, (size_t *)&textLen);
    int16_t     x          = luaL_optinteger(L, 2, game_state.x);
    int16_t     y          = luaL_optinteger(L, 3, game_state.y);
    int16_t     paletteIdx = luaL_optinteger(L, 4, game_state.color);

    game_state.x = draw_text(text, x, y, paletteIdx, true);
    game_state.y += 7;

    lua_pushnumber(L, (int16_t)game_state.x);
    return 1;
}

static const luaL_Reg funcs[] = {
    {"cls", aq8_cls},
    {"t", aq8_time},
    {"time", aq8_time},
    {"pset", aq8_pset},
    {"pal", aq8_pal},
    {"palt", aq8_palt},
    {"spr", aq8_spr},
    {"print", aq8_print},
    {NULL, NULL},
};

void mylib_register(lua_State *L) {
    lua_pushglobaltable(L);
    luaL_setfuncs(L, funcs, 0);
}
