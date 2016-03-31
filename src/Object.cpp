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
#include "Object.hpp"


namespace Luapp {

    static void DumpObject(std::ostream &out, lua_State *lua, int index, int level, bool indent)
    {
        for(int i = 0; i < indent ? level : 0; ++ i){
            out << "    ";
        }

        switch(lua_type(lua, index)){
            case LUA_TNIL:
                out << "nil";
                break;
            case LUA_TBOOLEAN:
                out << (lua_toboolean(lua, -1) ? "True" : "False");
                break;
            case LUA_TNUMBER:
                out << lua_tonumber(lua, index);
                break;
            case LUA_TSTRING:
                out << "'" << lua_tostring(lua, index) << "'";
                break;
            case LUA_TTABLE:
                out << "[table] " << lua_topointer(lua, index) << std::endl;
                if(level != 0){
                    break;
                }

                lua_pushnil(lua);
                while(lua_next(lua, -2)){
                    DumpObject(out, lua, -2, level + 1, true);
                    out << ": ";
                    DumpObject(out, lua, -1, level + 1, false);

                    out << std::endl;
                    lua_pop(lua, 1);
                }
                break;
            case LUA_TFUNCTION:
                out << "[function] " << lua_topointer(lua, index);
                break;
            case LUA_TUSERDATA:
                out << "[userdata] " << lua_topointer(lua, index);
                break;
            case LUA_TLIGHTUSERDATA:
                out << "[lightuserdata] " << lua_topointer(lua, index);
                break;
            default:
                break;
        }
    }

    std::ostream &operator << (std::ostream &out, const Object &obj)
    {
        if(obj.Valid()){
            obj.Push();
            DumpObject(out, obj.Lua(), -1, 0, true);
            lua_pop(obj.Lua(), 1);
        }
        else{
            out << "    obj is invalid!";
        }

        return out;
    }

    Object::Object():
        m_lua(nullptr),
        m_ref(LUA_REFNIL)
    {

    }

    Object::Object(lua_State *lua, int index):
        m_lua(lua)
    {
        lua_pushvalue(lua, index);
        m_ref = luaL_ref(m_lua, LUA_REGISTRYINDEX);
    }

    Object::~Object()
    {
        Release();
    }

    Object::Object(const Object &other)
    {
        m_lua = other.m_lua;
        lua_rawgeti(other.m_lua, LUA_REGISTRYINDEX, other.m_ref);
        m_ref = luaL_ref(m_lua, LUA_REGISTRYINDEX);
    }

    Object &Object::operator = (const Object &other)
    {
        if(this == &other){
            return *this;
        }

        Release();

        m_lua = other.m_lua;
        lua_rawgeti(other.m_lua, LUA_REGISTRYINDEX, other.m_ref);
        m_ref = luaL_ref(m_lua, LUA_REGISTRYINDEX);

        return *this;
    }

    Object::Object(Object &&other)
    {
        m_lua = other.m_lua;
        m_ref = other.m_ref;
        other.m_lua = nullptr;
        other.m_ref = LUA_NOREF;
    }

    Object &Object::operator = (Object &&other)
    {
        if(this == &other){
            return *this;
        }

        Release();

        m_lua = other.m_lua;
        m_ref = other.m_ref;
        other.m_lua = nullptr;
        other.m_ref = LUA_NOREF;

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
