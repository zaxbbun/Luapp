/* 
 * Copyright (c) 2012 ~ 2019 zaxbbun <zaxbbun@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LUAPP_STACK_HPP
#define LUAPP_STACK_HPP

#include <cstdint>
#include <string>
#include <lua.hpp>

namespace luapp { namespace detail {

    inline void stack_push(lua_State *lua, std::nullptr_t)
    {
        lua_pushnil(lua);
    }

    inline void stack_push(lua_State *lua, bool value)
    {
        lua_pushboolean(lua, value);
    }

    inline void stack_push(lua_State *lua, char value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, int8_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, uint8_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, int16_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, uint16_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, int32_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, uint32_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, int64_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, uint64_t value)
    {
        lua_pushinteger(lua, value);
    }

    inline void stack_push(lua_State *lua, float value)
    {
        lua_pushnumber(lua, value);
    }

    inline void stack_push(lua_State *lua, double value)
    {
        lua_pushnumber(lua, value);
    }

    inline void stack_push(lua_State *lua, long double value)
    {
        lua_pushnumber(lua, value);
    }

    inline void stack_push(lua_State *lua, const char *value)
    {
        lua_pushstring(lua, value);
    }

    inline void stack_push(lua_State *lua, const std::string &value)
    {
        lua_pushstring(lua, value.c_str());
    }

    inline void stack_push(lua_State *lua, lua_CFunction value)
    {
        lua_pushcfunction(lua, value);
    }

    inline void stack_push(lua_State *lua, void *value)
    {
        lua_pushlightuserdata(lua, value);
    }

    template<typename T> T stack_at(lua_State *lua, int index);

    template<> inline bool stack_at(lua_State *lua, int index)
    {
        return lua_toboolean(lua, index);
    }

    template<> inline char stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline int8_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline uint8_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline int16_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline uint16_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline int32_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline uint32_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline int64_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline uint64_t stack_at(lua_State *lua, int index)
    {
        return luaL_checkinteger(lua, index);
    }

    template<> inline float stack_at(lua_State *lua, int index)
    {
        return luaL_checknumber(lua, index);
    }

    template<> inline double stack_at(lua_State *lua, int index)
    {
        return luaL_checknumber(lua, index);
    }

    template<> inline long double stack_at(lua_State *lua, int index)
    {
        return luaL_checknumber(lua, index);
    }

    template<> inline const char *stack_at(lua_State *lua, int index)
    {
        return luaL_checkstring(lua, index);
    }

    template<> inline std::string stack_at(lua_State *lua, int index)
    {
        return luaL_checkstring(lua, index);
    }

    template<> inline lua_CFunction stack_at(lua_State *lua, int index)
    {
        return lua_tocfunction(lua, index);
    }

    template<> inline void *stack_at(lua_State *lua, int index)
    {
        return lua_touserdata(lua, index);
    }

    template<typename T> T stack_pop(lua_State *lua)
    {
        T value = stack_at<T>(lua, -1);
        lua_pop(lua, 1);
        return value;
    }

    class StackPop
    {
        public:
            StackPop(lua_State *lua, int size):
                lua_(lua),
                size_(size) { }

            ~StackPop()
            {
                lua_pop(lua_, size_);
            }

        private:
            lua_State *lua_;
            int       size_;
    };

    class StackGuard
    {
        public:
            StackGuard(lua_State *lua):
                lua_(lua),
                top_(lua_gettop(lua)) { }

            StackGuard(lua_State *lua, int top):
                lua_(lua),
                top_(top) { }

            ~StackGuard()
            {
                lua_settop(lua_, top_);
            }

            int value() const
            {
                return top_;
            }

        private:
            lua_State *lua_;
            int        top_;
    };

} }

#endif
