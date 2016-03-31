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

#include "type_proxy.hpp"
#include "class_meta.hpp"

namespace luapp { namespace detail {

    namespace matcher {

        bool object_matcher(lua_State *lua, int index, size_t type_hash, bool to_const)
        {
            if(!lua_isuserdata(lua, index) || !lua_getmetatable(lua, index)){
                return false;
            }

            lua_pushstring(lua, "__luapp_object_metatable");
            lua_rawget(lua, LUA_REGISTRYINDEX);

            bool equal = lua_rawequal(lua, -1, -2);
            lua_pop(lua, 2);

            if(!equal){
                return false;
            }

            ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, index));
            return obj_meta->Match(type_hash, to_const);
        }

    }

} }
