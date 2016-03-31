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

#ifndef LUAPP_FUNCTION_HPP
#define LUAPP_FUNCTION_HPP

#include <functional>

#include "Scope.hpp"
#include "TypeProxy.hpp"


namespace Luapp { namespace detail {

    template<typename F> struct function_traits;

    template<typename Ret, typename ...Args>
    struct function_traits<std::function<Ret (Args...)>>
    {
        using result_type = Ret;
        using arg_type_list = type_list<Args...>;
    };

    template<typename Ret, typename ...Args>
    struct function_traits<Ret (*)(Args...)>
    {
        using result_type = Ret;
        using arg_type_list = type_list<Args...>;
    };

    template<typename Ret, typename Class, typename ...Args>
    struct function_traits<Ret (Class:: *)(Args...)>
    {
        using result_type = Ret;
        using arg_type_list = type_list<Class &, Args...>;
    };

    template<typename Ret, typename Class, typename ...Args>
    struct function_traits<Ret (Class:: *)(Args...) const>
    {
        using result_type = Ret;
        using arg_type_list = type_list<const Class &, Args...>;
    };

    template<typename index_list, bool member_function, bool return_void> struct call_traits;

    template<uint32_t ...indices>
    struct call_traits<index_list<indices...>, false, true>
    {
        template<typename F, typename traits = function_traits<F>>
        static void call(lua_State *lua, F f)
        {
            using arg_type_list = typename traits::arg_type_list;

            f(
                type_proxy<
                    typename type_list_at<arg_type_list, indices>::type
                >::to_cpp(lua, indices + 1) ...
            );
        }
    };

    template<uint32_t ...indices>
    struct call_traits<index_list<indices...>, false, false>
    {
        template<typename F, typename traits = function_traits<F>>
        static void call(lua_State *lua, F f)
        {
            using arg_type_list = typename traits::arg_type_list;
            using result_type = typename traits::result_type;

            result_type result = f(
                type_proxy<
                    typename type_list_at<arg_type_list, indices>::type
                >::to_cpp(lua, indices + 1) ...
            );

            type_proxy<result_type>::to_lua(lua, result);
        }
    };

    template<uint32_t this_index, uint32_t ...indices>
    struct call_traits<index_list<this_index, indices...>, true, true>
    {
        template<typename F, typename traits = function_traits<F>>
        static void call(lua_State *lua, F f)
        {
            using arg_type_list = typename traits::arg_type_list;
            using class_t = typename type_list_at<arg_type_list, this_index>::type;

            class_t obj = type_proxy<class_t>::to_cpp(lua, this_index + 1);
            (obj.*f)(
                type_proxy<
                    typename type_list_at<arg_type_list, indices>::type
                >::to_cpp(lua, indices + 1) ...
            );
        }
    };

    template<uint32_t this_index, uint32_t ...indices>
    struct call_traits<index_list<this_index, indices...>, true, false>
    {
        template<typename F, typename traits = function_traits<F>>
        static void call(lua_State *lua, F f)
        {
            using arg_type_list = typename traits::arg_type_list;
            using class_t = typename type_list_at<arg_type_list, this_index>::type;
            using result_type = typename traits::result_type;

            class_t obj = type_proxy<class_t>::to_cpp(lua, this_index + 1);
            result_type result = (obj.*f)(
                type_proxy<
                    typename type_list_at<arg_type_list, indices>::type
                >::to_cpp(lua, indices + 1) ...
            );

            type_proxy<result_type>::to_lua(lua, result);
        }
    };

    template<typename type_list, int curr = 0, int end = type_list::size>
    struct type_list_match
    {
        static bool match(lua_State *lua, int index)
        {
            return type_proxy<
                    typename type_list_at<type_list, curr>::type
                >::match(lua, index) &&
                type_list_match<type_list, curr + 1, end>::match(lua, index + 1);
        }
    };

    template<typename type_list, int end>
    struct type_list_match<type_list, end, end>
    {
        static bool match(lua_State * /*lua*/, int /*index*/)
        {
            return true;
        }
    };

    class FunctionBase
    {
        public:
            struct OverloadsFilter {
                bool find;
                int index;
                const FunctionBase *overloads[0x0F];
            };

            static void RegisterFunction(lua_State *lua, FunctionBase *func);
            static void CreateFunction(lua_State *lua, FunctionBase *func);
            static int DestroyFunction(lua_State *lua);
            static int Call(lua_State *lua);

            FunctionBase(const std::string &name, const std::string &signature);
            virtual ~FunctionBase() = default;

            std::string Name() const {
                return m_name;
            }

            std::string Signature() const {
                return m_signature;
            }

            FunctionBase *Next() const {
                return m_next;
            }

            virtual int operator () (lua_State *lua, OverloadsFilter &filter) const = 0;

        private:
            std::string m_name;
            std::string m_signature;
            FunctionBase *m_next;
    };

    template<typename F>
    class Function: public FunctionBase
    {
        public:
            using traits = function_traits<F>;
            using result_type = typename traits::result_type;
            using arg_type_list = typename traits::arg_type_list;
            using arg_index_list = typename type_list_index<arg_type_list>::type;

            Function(const std::string &name, F f):
                FunctionBase(name, typeid(F).name()),
                m_func(f) { }
            ~Function() = default;

            int operator () (lua_State *lua, OverloadsFilter &filter) const override
            {
                int argc = lua_gettop(lua);

                bool match = argc == arg_type_list::size &&
                    type_list_match<arg_type_list>::match(lua, 1);
                if(!match){
                    filter.overloads[filter.index++] = this;
                    FunctionBase *next = Next();
                    return next ? (*next)(lua, filter) : 0;
                }

                filter.find = true;
                call_traits<
                    arg_index_list,
                    std::is_member_function_pointer<F>::value,
                    std::is_void<result_type>::value
                >::template call<F, traits>(lua, m_func);

                return lua_gettop(lua) - argc;
            }

        private:
            F m_func;
    };

    class FunctionRegistry: public Registry
    {
        public:
            FunctionRegistry(FunctionBase *func):
                m_func(func){ }
            ~FunctionRegistry() = default;

            void Register(lua_State *lua) const override
            {
                FunctionBase::RegisterFunction(lua, m_func);
            }

        private:
            FunctionBase *m_func;
    };

} }


namespace Luapp {

    template<typename F>
    std::shared_ptr<detail::FunctionRegistry> def(const std::string &name, F f)
    {
        return std::make_shared<detail::FunctionRegistry>(new detail::Function<F>(name, f));
    };

}

#endif
