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

#include "module.hpp"
#include "scope.hpp"

namespace luapp { namespace detail {

    Module::Module(lua_State *lua, const std::string &name):
        lua_(lua),
        name_(name)
    {

    }

    void Module::operator [] (const std::shared_ptr<detail::Registry> &reg)
    {
        detail::ScopeRegistry scope(name_);
        scope.AddInner(reg);

        lua_rawgeti(lua_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
        scope.Register(lua_);
        lua_pop(lua_, 1);
    }

} }

namespace luapp {

    detail::Module module(lua_State *lua, const std::string &name)
    {
        return detail::Module(lua, name);
    }

}
