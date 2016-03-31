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

#ifndef LUAPP_ENUM_HPP
#define LUAPP_ENUM_HPP

#include "scope.hpp"

namespace luapp { namespace detail {

    class EnumRegistry: public Registry
    {
        public:
            EnumRegistry(const std::string &name, int value):
                name_(name),
                value_(value){ }
            ~EnumRegistry() = default;

            void Register(lua_State *lua) const override
            {
                lua_pushstring(lua, name_.c_str());
                lua_pushinteger(lua, value_);
                lua_rawset(lua, -3);
            };

        private:
            std::string name_;
            int value_;
    };

    class Enum: public Scope
    {
        public:
            Enum(const std::string &name):
                Scope(name){ }
            ~Enum() = default;

            Enum &def(const std::string &name, int value)
            {
                (*this)[std::make_shared<EnumRegistry>(name, value)];
                return *this;
            }

        private:
            using Scope::operator [];
    };

} }

namespace luapp {

    detail::Enum enum_(const std::string &name);

}

#endif
