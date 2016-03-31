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
#include "Stack.hpp"


namespace Luapp { namespace detail {

    std::string DumpStack(lua_State *lua)
    {
        std::ostringstream out;
        out << "dump stack: " << std::endl;

        int size = lua_gettop(lua);
        for(int i = 0; i < size; ++ i){
            out << "    " << luaL_typename(lua, i) << " ";
            switch(lua_type(lua, i)){
                case LUA_TNIL:
                    out << "nil";
                    break;
                case LUA_TBOOLEAN:
                    out << (lua_toboolean(lua, -1) ? "True" : "False");
                    break;
                case LUA_TNUMBER:
                    out << lua_tonumber(lua, i);
                    break;
                case LUA_TSTRING:
                    out << "\"" << lua_tostring(lua, i) << "\"";
                    break;
                case LUA_TTABLE:
                    out << "[table] " << lua_topointer(lua, i);
                    break;
                case LUA_TFUNCTION:
                    out << "[function] " << lua_topointer(lua, i);
                    break;
                case LUA_TUSERDATA:
                    out << "[userdata] " << lua_topointer(lua, i);
                    break;
                case LUA_TLIGHTUSERDATA:
                    out << "[lightuserdata] " << lua_topointer(lua, i);
                    break;
                default:
                    break;
            }

            out << std::endl;
        }

        return out.str();
    }

} }
