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

#ifndef LUAPP_SCOPE_HPP
#define LUAPP_SCOPE_HPP

#include <string>
#include "registry.hpp"

namespace luapp { namespace detail {

    class ScopeRegistry: public Registry
    {
        public:
            ScopeRegistry(const std::string &name = "");
            ~ScopeRegistry() = default;

            void AddInner(const std::shared_ptr<Registry> &reg);
            void Register(lua_State *lua) const override;

        private:
            std::string name_;
            std::shared_ptr<Registry> inner_;
    };

    class Scope
    {
        public:
            Scope(const std::string &name = "");
            ~Scope() = default;

            operator std::shared_ptr<Registry> () const;
            Scope &operator , (const std::shared_ptr<Registry> &reg);
            Scope &operator [] (const std::shared_ptr<Registry> &reg);

        private:
            std::shared_ptr<ScopeRegistry> reg_;
    };

} }

namespace luapp {

    detail::Scope scope_(const std::string &name = "");

}

#endif
