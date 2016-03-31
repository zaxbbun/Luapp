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

#include "scope.hpp"

namespace luapp { namespace detail {

    ScopeRegistry::ScopeRegistry(const std::string &name):
        name_(name)
    {

    }

    void ScopeRegistry::AddInner(const std::shared_ptr<Registry> &reg)
    {
        if(!inner_){
            inner_ = reg;
        }
        else{
            inner_->Join(reg);
        }
    }

    void ScopeRegistry::Register(lua_State *lua) const
    {
        if(!name_.empty()){
            lua_getfield(lua, -1, name_.c_str());
            if(!lua_istable(lua, -1)){
                lua_pop(lua, 1);
                lua_newtable(lua);
                lua_pushstring(lua, name_.c_str());
                lua_pushvalue(lua, -2);
                lua_settable(lua, -4);
            }
        }

        for(const std::shared_ptr<Registry> *reg = &inner_; *reg; reg = &(*reg)->next_){
            (*reg)->Register(lua);
        }

        if(!name_.empty()){
            lua_pop(lua, 1);
        }
    }

    Scope::Scope(const std::string &name):
        reg_(std::make_shared<ScopeRegistry>(name))
    {

    }

    Scope::operator std::shared_ptr<Registry> () const
    {
        return reg_;
    }

    Scope &Scope::operator , (const std::shared_ptr<Registry> &reg)
    {
        reg_->Join(reg);
        return *this;
    }

    Scope &Scope::operator [] (const std::shared_ptr<Registry> &reg)
    {
        reg_->AddInner(reg);
        return *this;
    }

} }

namespace luapp {

    detail::Scope scope_(const std::string &name)
    {
        return detail::Scope(name);
    }

}
