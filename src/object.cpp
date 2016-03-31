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

#include <sstream>
#include "object.hpp"

namespace luapp {

    std::ostream &operator << (std::ostream &out, const Object &obj)
    {
        Object require = global(obj.Lua())["require"];
        Object pformat = require.call<Object>("pprint")["pformat"];

        Object options = make_table(obj.Lua());
        options["show_all"] = true;

        out << pformat.call<std::string>(obj, options);
        return out;
    }

    Object::Object():
        lua_(nullptr),
        ref_(LUA_REFNIL)
    {

    }

    Object::Object(lua_State *lua, int index):
        lua_(lua)
    {
        lua_pushvalue(lua, index);
        ref_ = luaL_ref(lua_, LUA_REGISTRYINDEX);
    }

    Object::~Object()
    {
        Release();
    }

    Object::Object(const Object &other)
    {
        lua_ = other.lua_;
        lua_rawgeti(other.lua_, LUA_REGISTRYINDEX, other.ref_);
        ref_ = luaL_ref(lua_, LUA_REGISTRYINDEX);
    }

    Object &Object::operator = (const Object &other)
    {
        if(this == &other){
            return *this;
        }

        Release();

        lua_ = other.lua_;
        lua_rawgeti(other.lua_, LUA_REGISTRYINDEX, other.ref_);
        ref_ = luaL_ref(lua_, LUA_REGISTRYINDEX);

        return *this;
    }

    Object::Object(Object &&other)
    {
        lua_ = other.lua_;
        ref_ = other.ref_;
        other.lua_ = nullptr;
        other.ref_ = LUA_NOREF;
    }

    Object &Object::operator = (Object &&other)
    {
        if(this == &other){
            return *this;
        }

        Release();

        lua_ = other.lua_;
        ref_ = other.ref_;
        other.lua_ = nullptr;
        other.ref_ = LUA_NOREF;

        return *this;
    }

    Object global(lua_State *lua)
    {
        detail::StackPop pop(lua, 1);
        lua_rawgeti(lua, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
        return Object(lua, -1);
    }

    Object registry(lua_State *lua)
    {
        detail::StackPop pop(lua, 1);
        lua_pushvalue(lua, LUA_REGISTRYINDEX);
        return Object(lua, -1);
    }

    Object make_table(lua_State *lua)
    {
        detail::StackPop pop(lua, 1);
        lua_newtable(lua);
        return Object(lua, -1);
    };

}
