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

#ifndef LUAPP_CLASS_META_HPP
#define LUAPP_CLASS_META_HPP

#include <map>
#include <vector>
#include <functional>

#include "object.hpp"

namespace luapp { namespace detail {

    void PushClassMetatable(lua_State *lua);

    class ClassMeta
    {
        public:
            typedef std::function<void *(void *)> CastFunction;

            ClassMeta(lua_State *lua, const std::string &name, size_t type_hash);
            ~ClassMeta() = default;

            lua_State *Lua() const
            {
                return index_table_.Lua();
            }

            Object Document() const;

            size_t TypeHash() const
            {
                return type_hash_;
            }

            void AddBase(const ClassMeta &base, const CastFunction &func)
            {
                base_list_.push_back({base, func});
            }

            void PushIndexTable() const
            {
                index_table_.Push();
            }

            void PushNewIndexTable() const
            {
                newindex_table_.Push();
            }

            bool FindIndex(int key) const;
            bool FindNewIndex(int key) const;

            bool Convertable(size_t type_hash) const;
            void *Cast(void *ptr, size_t type_hash) const;

        private:
            struct BaseClass {
                const ClassMeta &meta;
                CastFunction cast;
            };

            std::string name_;
            size_t type_hash_;

            Object index_table_;
            Object newindex_table_;

            std::vector<BaseClass> base_list_;
    };

    class ClassMetaMgr
    {
        public:
            ClassMetaMgr() = default;
            ~ClassMetaMgr() = default;

            void AddClassMeta(const ClassMeta *class_meta);
            const ClassMeta *GetClassMeta(size_t type_hash) const;

        private:
            std::map<size_t, const ClassMeta *> class_map_;
    };

    ClassMetaMgr &GetClassMetaMgr(lua_State *lua);

} }

#endif
