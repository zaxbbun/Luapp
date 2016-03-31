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

#ifndef LUAPP_TYPE_PROXY_HPP
#define LUAPP_TYPE_PROXY_HPP

#include <tuple>
#include <typeinfo>

#include "Stack.hpp"
#include "ObjectMeta.hpp"


namespace Luapp { namespace detail {

    void CreateObjectMeta(lua_State *lua, size_t type_hash, const std::string &type_name,
        IObjectHolder *holder);

    template<typename ...Types> struct type_list
    {
        enum { size = sizeof ...(Types) };
    };

    template<typename type_list, uint32_t index> struct type_list_at;

    template<typename Type0, typename ...TypeN, uint32_t index>
    struct type_list_at<type_list<Type0, TypeN...>, index>:
        type_list_at<type_list<TypeN...>, index - 1>
    {

    };

    template<typename Type0, typename ...TypeN>
    struct type_list_at<type_list<Type0, TypeN...>, 0>
    {
        using type = Type0;
    };

    template<typename type_list> struct make_tuple;

    template<typename ...Types>
    struct make_tuple<type_list<Types...>>
    {
        using type = std::tuple<Types...>;
    };

    template<uint32_t ...indices> struct index_list;

    template<uint32_t curr, uint32_t end, uint32_t ...indices>
    struct make_range: make_range<curr + 1, end, indices ..., curr>
    {

    };

    template<uint32_t end, uint32_t ...indices>
    struct make_range<end, end, indices...>
    {
        using type = index_list<indices...>;
    };

    template<typename type_list>
    struct type_list_index
    {
        using type = typename make_range<0, type_list::size>::type;
    };

    namespace matcher {

        struct nil_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_isnil(lua, index);
            }
        };

        struct bool_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_isboolean(lua, index);
            }
        };

        struct number_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_isnumber(lua, index);
            }
        };

        struct string_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_isstring(lua, index);
            }
        };

        struct lightuserdata_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_islightuserdata(lua, index);
            }
        };

        struct function_matcher
        {
            static bool match(lua_State *lua, int index)
            {
                return lua_iscfunction(lua, index);
            }
        };

        bool object_matcher(lua_State *lua, int index, size_t type_hash, bool to_const);

    }

    template<typename T, typename type_matcher>
    struct native_proxy
    {
        static bool match(lua_State *lua, int index)
        {
            return type_matcher::match(lua, index);
        };

        static void to_lua(lua_State *lua, T value)
        {
            stack_push(lua, value);
        }

        static T to_cpp(lua_State *lua, int index)
        {
            return stack_at<T>(lua, index);
        }
    };

    template<typename T>
    struct enum_proxy
    {
        static bool match(lua_State *lua, int index)
        {
            return lua_isinteger(lua, index);
        }

        static void to_lua(lua_State *lua, T value)
        {
            lua_pushinteger(lua, value);
        };

        static T to_cpp(lua_State *lua, int index)
        {
            return static_cast<T>(lua_tointeger(lua, index));
        }
    };

    template<typename T>
    struct value_proxy
    {
        static const size_t type_hash;
        static const std::string type_name;

        static bool match(lua_State *lua, int index)
        {
            return matcher::object_matcher(lua, index, type_hash, true);
        };

        static void to_lua(lua_State *lua, T value)
        {
            IObjectHolder *holder = new ValueHolder<T>(value);
            CreateObjectMeta(lua, type_hash, type_name, holder);
        }

        static T to_cpp(lua_State *lua, int index)
        {
            ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, index));
            return *static_cast<T *>(obj_meta->GetObject(type_hash));
        }
    };

    template<typename T>
    const size_t value_proxy<T>::type_hash = typeid(T).hash_code();

    template<typename T>
    const std::string value_proxy<T>::type_name = typeid(T).name();

    template<typename T>
    struct reference_proxy
    {
        using raw_type = typename std::remove_reference<T>::type;

        static const size_t type_hash;
        static const std::string type_name;

        static bool match(lua_State *lua, int index)
        {
            return matcher::object_matcher(lua, index, type_hash,
                std::is_const<raw_type>::value);
        }

        static void to_lua(lua_State *lua, T value)
        {
            IObjectHolder *holder = new PointerHolder<raw_type *>(&value, false);
            CreateObjectMeta(lua, type_hash, type_name, holder);
        }

        static T to_cpp(lua_State *lua, int index)
        {
            ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, index));
            return *static_cast<raw_type *>(obj_meta->GetObject(type_hash));
        }
    };

    template<typename T>
    const size_t reference_proxy<T>::type_hash = typeid(raw_type).hash_code();

    template<typename T>
    const std::string reference_proxy<T>::type_name = typeid(raw_type).name();

    template<typename T>
    struct pointer_proxy
    {
        using raw_type = typename std::remove_pointer<T>::type;

        static const size_t type_hash;
        static const std::string type_name;

        static bool match(lua_State *lua, int index)
        {
            return lua_isnil(lua, index) || matcher::object_matcher(lua, index,
                type_hash, std::is_const<raw_type>::value);
        }

        static void to_lua(lua_State *lua, T value)
        {
            if(value == nullptr){
                lua_pushnil(lua);
                return;
            }

            IObjectHolder *holder = new PointerHolder<T>(value, false);
            CreateObjectMeta(lua, type_hash, type_name, holder);
        }

        static T to_cpp(lua_State *lua, int index)
        {
            ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, index));
            return obj_meta ? static_cast<T>(obj_meta->GetObject(type_hash)) : nullptr;
        }
    };

    template<typename T>
    const size_t pointer_proxy<T>::type_hash = typeid(raw_type).hash_code();

    template<typename T>
    const std::string pointer_proxy<T>::type_name = typeid(raw_type).name();

    struct class_meta_proxy
    {
        static bool match(lua_State *lua, int index)
        {
            return lua_isuserdata(lua, index);
        }

        static void to_lua(lua_State * /*lua*/, const ClassMeta & /*value*/)
        {

        }

        static const ClassMeta &to_cpp(lua_State *lua, int index)
        {
            return *static_cast<const ClassMeta *>(lua_touserdata(lua, index));
        }
    };

    template<typename Case0, typename ...CaseN>
    struct switch_
    {
        using type = typename std::conditional<Case0::value, typename Case0::type,
            typename switch_<CaseN...>::type>::type;
    };

    template<typename Case0>
    struct switch_<Case0>
    {
        using type = typename std::conditional<Case0::value, typename Case0::type,
            void>::type;
    };

    template<bool flag, typename T>
    struct case_
    {
        enum { value = flag };
        using type = T;
    };

    template<typename T>
    struct default_
    {
        enum { value = true };
        using type = T;
    };

    template<typename T>
    struct is_number: std::integral_constant<bool,
        std::is_integral<T>::value ||
        std::is_floating_point<T>::value
    > { };

    template<typename T>
    struct is_reference_to_number: std::integral_constant<bool,
        std::is_reference<T>::value &&
        is_number<
            typename std::remove_const<typename std::remove_reference<T>::type>::type
        >::value
    > { };

    template<typename T>
    struct is_pointer_to_number: std::integral_constant<bool,
        std::is_pointer<T>::value &&
        is_number<
            typename std::remove_const<typename std::remove_pointer<T>::type>::type
        >::value
    > { };

    template<typename T>
    struct type_proxy:
        switch_<
            case_<detail::is_number<T>::value, native_proxy<T, matcher::number_matcher>>,
            case_<
                detail::is_reference_to_number<T>::value,
                native_proxy<
                    typename std::remove_const<typename std::remove_reference<T>::type>::type,
                    matcher::number_matcher
                >
            >,
            case_<
                std::is_same<T, std::nullptr_t>::value | detail::is_pointer_to_number<T>::value,
                native_proxy<
                    typename std::remove_const<typename std::remove_pointer<T>::type>::type,
                    matcher::number_matcher
                >
            >,
            case_<std::is_reference<T>::value, reference_proxy<T>>,
            case_<std::is_pointer<T>::value, pointer_proxy<T>>,
            case_<std::is_enum<T>::value, enum_proxy<T>>,
            default_<value_proxy<T>>
        >::type
    {

    };

    template<> struct type_proxy<std::nullptr_t>: native_proxy<std::nullptr_t, matcher::nil_matcher> { };

    template<> struct type_proxy<void *>: native_proxy<void *, matcher::lightuserdata_matcher> { };
    template<> struct type_proxy<void *const>: type_proxy<void *> { };

    template<> struct type_proxy<bool>: native_proxy<bool, matcher::bool_matcher> { };
    template<> struct type_proxy<const bool &>: type_proxy<bool> { };
    template<> struct type_proxy<const bool *>: type_proxy<bool> { };

    template<> struct type_proxy<const char *>: native_proxy<const char *, matcher::string_matcher> { };
    template<> struct type_proxy<const char *const>: type_proxy<const char *> { };
    template<> struct type_proxy<const char *&>: type_proxy<const char *> { };

    template<uint32_t N> struct type_proxy<const char [N]>: type_proxy<const char *> { };
    template<uint32_t N> struct type_proxy<const char (&)[N]>: type_proxy<const char *> { };

    template<> struct type_proxy<std::string>: native_proxy<std::string, matcher::string_matcher>{ };
    template<> struct type_proxy<const std::string &>: type_proxy<std::string> { };
    template<> struct type_proxy<const std::string *>: type_proxy<std::string> { };

    template<> struct type_proxy<lua_CFunction>: native_proxy<
        lua_CFunction, matcher::function_matcher> { };

    template<> struct type_proxy<const ClassMeta &>: class_meta_proxy { };

    template<typename tuple_type, typename index_list> struct tuple_proxy { };

    template<typename ...Types, uint32_t ...indices>
    struct tuple_proxy<std::tuple<Types...>, index_list<indices...>>
    {
        using tuple_type = std::tuple<Types...>;

        template<uint32_t curr, uint32_t end>
        struct proxy_helper
        {
            static bool match(lua_State *lua, int index)
            {
                return type_proxy<
                    typename std::tuple_element<curr, tuple_type>::type
                >::match(lua, index) && proxy_helper<curr + 1, end>::match(lua, index + 1);
            }

            static void to_lua(lua_State *lua, const tuple_type &value)
            {
                type_proxy<
                    typename std::tuple_element<curr, tuple_type>::type
                >::to_lua(lua, std::get<curr>(value));
                proxy_helper<curr + 1, end>::to_lua(lua, value);
            }
        };

        template<uint32_t end>
        struct proxy_helper<end, end>
        {
            static bool match(lua_State * /*lua*/, int /*index*/)
            {
                return true;
            }

            static void to_lua(lua_State * /*lua*/, const tuple_type &/*value*/)
            {

            }
        };

        static bool match(lua_State *lua, int index)
        {
            return proxy_helper<0, std::tuple_size<tuple_type>::value>::match(lua, index);
        }

        static void to_lua(lua_State *lua, const tuple_type &value)
        {
            proxy_helper<0, std::tuple_size<tuple_type>::value>::to_lua(lua, value);
        }

        static tuple_type to_cpp(lua_State *lua, int index)
        {
            return std::make_tuple(type_proxy<
                typename std::tuple_element<indices, tuple_type>::type
            >::to_cpp(lua, index + indices) ...);
        }
    };

    template<typename ...Types>
    struct type_proxy<std::tuple<Types...>>: tuple_proxy<std::tuple<Types...>,
        typename type_list_index<type_list<Types...>>::type>
    {

    };

    template<typename ...Types>
    struct type_proxy<const std::tuple<Types...> &>: type_proxy<std::tuple<Types...>> { };

    template<typename ...Types>
    struct type_proxy<const std::tuple<Types...> *>: type_proxy<std::tuple<Types...>> { };

} }

#endif
