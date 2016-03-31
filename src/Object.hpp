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

#ifndef LUAPP_OBJECT_HPP
#define LUAPP_OBJECT_HPP

#include <cstdlib>

#include "TypeProxy.hpp"
#include "Traceback.hpp"


namespace Luapp {

    namespace detail {

        inline void push_args(lua_State * /*lua*/) { }

        template<typename Arg0, typename ...Args>
        static void push_args(lua_State *lua, Arg0 &&arg0, Args&& ...args)
        {
            type_proxy<Arg0>::to_lua(lua, std::forward<Arg0>(arg0));
            push_args(lua, std::forward<Args>(args)...);
        }

        template<bool return_void> struct call_function_aux;

        template<>
        struct call_function_aux<true>
        {
            template<typename Ret>
            static void call(lua_State *lua, int base, int argc)
            {
                lua_pushcfunction(lua, Traceback);
                lua_insert(lua, base + 1);

                if(lua_pcall(lua, argc, 0, base + 1)){
                    fprintf(stderr, "\n%s\n", lua_tostring(lua, -1));
                    std::abort();
                }

                lua_settop(lua, base);
            }
        };

        template<>
        struct call_function_aux<false>
        {
            template<typename Ret>
            static Ret call(lua_State *lua, int base, int argc)
            {
                lua_pushcfunction(lua, Traceback);
                lua_insert(lua, base + 1);

                if(lua_pcall(lua, argc, LUA_MULTRET, base + 1)){
                    fprintf(stderr, "\n%s\n", lua_tostring(lua, -1));
                    std::abort();
                }

                StackPop pop(lua, lua_gettop(lua) - base);
                return type_proxy<Ret>::to_cpp(lua, base + 2);
            }
        };

    }

    template<typename Key> class Node;

    class Object
    {
        template<typename Key> friend class Node;
        friend std::ostream &operator << (std::ostream &out, const Object &obj);

        public:
            Object();
            Object(lua_State *lua, int index);
            ~Object();

            Object(const Object &other);
            Object &operator = (const Object &other);

            Object(Object &&other);
            Object &operator = (Object &&other);

            lua_State *Lua() const
            {
                return m_lua;
            }

            bool Valid() const
            {
                return m_lua && m_ref != LUA_NOREF;
            }

            bool IsNil() const
            {
                return m_lua && m_ref == LUA_REFNIL;
            }

            void Push() const
            {
                lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_ref);
            }

            void Release()
            {
                if(Valid()){
                    luaL_unref(m_lua, LUA_REGISTRYINDEX, m_ref);
                    m_lua = nullptr;
                    m_ref = LUA_NOREF;
                }
            }

            template<typename T> T GetValue(T defval) const
            {
                detail::StackPop pop(m_lua, 1);
                Push();

                if(detail::type_proxy<T>::match(m_lua, -1)){
                    return detail::type_proxy<T>::to_cpp(m_lua, -1);
                }
                else{
                    return defval;
                }
            }

            template<typename T> operator T () const
            {
                return GetValue(T());
            }

            template<typename T> Node<T> operator [] (T &&key) const
            {
                return Node<T>(*this, std::forward<T>(key));
            }

            template<typename Ret = void, typename ...Args>
            Ret call(Args&& ...args)
            {
                int base = lua_gettop(m_lua);
                Push();

                if(!lua_isfunction(m_lua, -1)){
                    fprintf(stderr, "function expected!, got %s\n",
                        luaL_typename(m_lua, -1));
                    std::abort();
                }

                detail::push_args(m_lua, std::forward<Args>(args)...);
                return detail::call_function_aux<
                        std::is_void<Ret>::value
                    >::template call<Ret>(m_lua, base, sizeof ...(Args));
            }

            template<typename Ret = void, typename ...Args>
            Ret call_member(const std::string &func, Args&& ...args)
            {
                int base = lua_gettop(m_lua);
                Push();

                lua_getfield(m_lua, -1, func.c_str());
                if(!lua_isfunction(m_lua, -1)){
                    fprintf(stderr, "member function expected!, got %s\n",
                        luaL_typename(m_lua, -1));
                    std::abort();
                }

                lua_insert(m_lua, -2);
                detail::push_args(m_lua, std::forward<Args>(args)...);
                return detail::call_function_aux<
                        std::is_void<Ret>::value
                    >::template call<Ret>(m_lua, base, sizeof ...(Args) + 1);
            }

        private:
            lua_State *m_lua;
            int m_ref;
    };

    namespace detail {

        struct object_proxy
        {
            static bool match(lua_State * /*lua*/, int /*index*/)
            {
                return true;
            }

            static void to_lua(lua_State * /*lua*/, const Object &value)
            {
                value.Push();
            }

            static Object to_cpp(lua_State *lua, int index)
            {
                return Object(lua, index);
            }

        };

        template<> struct type_proxy<Object>: object_proxy { };
        template<> struct type_proxy<Object &>: object_proxy { };
        template<> struct type_proxy<const Object &>: object_proxy { };

    }

    template<typename Key>
    class Node
    {
        public:
            Node(const Object &table, const Key &key):
                m_table(table),
                m_key(key) { }
            ~Node() = default;

            operator Object() const
            {
                detail::StackPop pop(m_table.m_lua, 2);

                m_table.Push();
                detail::stack_push(m_table.m_lua, m_key);
                lua_gettable(m_table.m_lua, -2);

                return Object(m_table.m_lua, -1);
            };

            template<typename T> operator T () const
            {
                return static_cast<const Object &>(*this);
            }

            template<typename T> Node<T> operator [] (T &&key) const
            {
                return static_cast<const Object &>(*this)[std::forward<T>(key)];
            }

            template<typename T> void operator = (const T &value)
            {
                lua_State *lua = m_table.m_lua;
                m_table.Push();

                detail::stack_push(lua, m_key);
                detail::type_proxy<T>::to_lua(lua, value);
                lua_settable(lua, -3);

                lua_pop(lua, 1);
            }

        private:
            Object m_table;
            Key m_key;
    };

    Object global(lua_State *lua);
    Object registry(lua_State *lua);
    Object make_table(lua_State *lua);

}

#endif
