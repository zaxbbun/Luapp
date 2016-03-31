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

#include "class.hpp"
#include "class_meta.hpp"

namespace luapp { namespace detail {

    int PropertyTag(lua_State *lua)
    {
        luaL_error(lua, "[luapp] cannot call PropertyTag directly");
        return 0;
    }

    void ClassRegistry::Register(lua_State *lua) const
    {
        lua_pushstring(lua, name_.c_str());

        ClassMeta *class_meta = (ClassMeta *)lua_newuserdata(lua, sizeof(ClassMeta));
        new (class_meta) ClassMeta(lua, name_, type_hash_);

        for(const BaseInfo &base_info: base_list_){
            const ClassMeta *base = GetClassMetaMgr(lua).GetClassMeta(base_info.hash_code);
            if(!base){
                luaL_error(lua, "[luapp] one or more bases of %s have not been registered "
                    "or in bad order!", name_.c_str());
            }

            class_meta->AddBase(*base, base_info.cast);
        }

        PushClassMetatable(lua);
        lua_setmetatable(lua, -2);

        class_meta->PushIndexTable();
        index_.Register(lua);
        lua_pop(lua, 1);

        class_meta->PushNewIndexTable();
        newindex_.Register(lua);
        lua_pop(lua, 1);

        lua_rawset(lua, -3);
    }

} }
