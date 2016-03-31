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

#include "Object.hpp"


namespace Luapp { namespace detail {

    void PushClassMetatable(lua_State *lua);
    void CreateObjectMeta(const ClassMeta &class_meta, IObjectHolder *holder);
    void CreateObjectMeta(lua_State *lua, size_t type_hash, const std::string &type_name,
        IObjectHolder *holder);

    class ClassMeta
    {
        public:
            typedef std::function<void *(void *)> CastFunction;

            ClassMeta(lua_State *lua, const std::string &name, size_t type_hash);
            ~ClassMeta() = default;

            lua_State *Lua() const
            {
                return m_index_table.Lua();
            }

            size_t TypeHash() const
            {
                return m_type_hash;
            }

            void AddBase(const ClassMeta &base, const CastFunction &func)
            {
                m_base_list.push_back({base, func});
            }

            void PushIndexTable() const
            {
                m_index_table.Push();
            }

            void PushNewIndexTable() const
            {
                m_newindex_table.Push();
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

            std::string m_name;
            size_t m_type_hash;

            Object m_index_table;
            Object m_newindex_table;

            std::vector<BaseClass> m_base_list;
    };

    class ClassMetaMgr
    {
        public:
            ClassMetaMgr() = default;
            ~ClassMetaMgr() = default;

            void AddClassMeta(const ClassMeta *class_meta);
            const ClassMeta *GetClassMeta(size_t type_hash) const;

        private:
            std::map<size_t, const ClassMeta *> m_class_map;
    };

    ClassMetaMgr &GetClassMetaMgr(lua_State *lua);

} }

#endif
