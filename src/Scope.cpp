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

#include "Scope.hpp"


namespace Luapp { namespace detail {

    ScopeRegistry::ScopeRegistry(const std::string &name):
        m_name(name)
    {

    }

    void ScopeRegistry::AddInner(const std::shared_ptr<Registry> &reg)
    {
        if(!m_inner){
            m_inner = reg;
        }
        else{
            m_inner->Join(reg);
        }
    }

    void ScopeRegistry::Register(lua_State *lua) const
    {
        if(!m_name.empty()){
            lua_getfield(lua, -1, m_name.c_str());
            if(!lua_istable(lua, -1)){
                lua_pop(lua, 1);
                lua_newtable(lua);
                lua_pushstring(lua, m_name.c_str());
                lua_pushvalue(lua, -2);
                lua_settable(lua, -4);
            }
        }

        for(const std::shared_ptr<Registry> *reg = &m_inner; *reg; reg = &(*reg)->m_next){
            (*reg)->Register(lua);
        }

        if(!m_name.empty()){
            lua_pop(lua, 1);
        }
    }

    Scope::Scope(const std::string &name):
        m_reg(std::make_shared<ScopeRegistry>(name))
    {

    }

    Scope::operator std::shared_ptr<Registry> () const
    {
        return m_reg;
    }

    Scope &Scope::operator , (const std::shared_ptr<Registry> &reg)
    {
        m_reg->Join(reg);
        return *this;
    }

    Scope &Scope::operator [] (const std::shared_ptr<Registry> &reg)
    {
        m_reg->AddInner(reg);
        return *this;
    }

} }


namespace Luapp {

    detail::Scope scope_(const std::string &name)
    {
        return detail::Scope(name);
    }

}
