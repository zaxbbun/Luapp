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

#ifndef LUAPP_CLASS_HPP
#define LUAPP_CLASS_HPP

#include "Function.hpp"
#include "ClassMeta.hpp"


namespace Luapp { namespace detail {

    int PropertyTag(lua_State *lua);

    template<typename ClassT, typename ...Args>
    struct Constructor
    {
        static void Construct(const ClassMeta &class_meta, Args ...args)
        {
            IObjectHolder *holder = new PointerHolder<ClassT *>(new ClassT(args...), true);
            CreateObjectMeta(class_meta, holder);
        }
    };

    class ClassRegistry: public Registry
    {
        public:
            typedef std::function<void *(void *)> CastFunction;

            ClassRegistry(const std::string &name, size_t type_hash):
                m_name(name),
                m_type_hash(type_hash){ }
            ~ClassRegistry() = default;

            void AddBase(size_t type_hash, const CastFunction &cast)
            {
                m_base_list.push_back({type_hash, cast});
            }

            void AddIndex(const std::shared_ptr<Registry> &reg)
            {
                m_index.AddInner(reg);
            }

            void AddNewIndex(const std::shared_ptr<Registry> &reg)
            {
                m_newindex.AddInner(reg);
            }

            void Register(lua_State *lua) const override;

        private:
            struct BaseInfo {
                size_t hash_code;
                CastFunction cast;
            };

            std::string m_name;
            size_t m_type_hash;

            ScopeRegistry m_index;
            ScopeRegistry m_newindex;

            std::vector<BaseInfo> m_base_list;
    };

    class PropertyRegistry: public Registry
    {
        public:
            PropertyRegistry(FunctionBase *accessor):
                m_accessor(accessor){ }
            ~PropertyRegistry() = default;

            void Register(lua_State *lua) const override
            {
                lua_pushstring(lua, m_accessor->Name().c_str());
                FunctionBase::CreateFunction(lua, m_accessor);
                lua_pushcclosure(lua, PropertyTag, 1);
                lua_rawset(lua, -3);
            }

        private:
            FunctionBase *m_accessor;
    };

    template<typename ClassT, typename ...Bases>
    class Class
    {
        public:
            Class(const std::string &name):
                m_class_reg(std::make_shared<ClassRegistry>(name, typeid(ClassT).hash_code()))
            {
                AddBaseList(type_list<Bases...>());
            }

            ~Class() = default;

            operator std::shared_ptr<Registry> () const
            {
                return m_class_reg;
            }

            Class &operator , (const std::shared_ptr<Registry> &reg)
            {
                m_class_reg->Join(reg);
                return *this;
            }

            Class &operator [] (const std::shared_ptr<Registry> &reg)
            {
                m_class_reg->AddIndex(reg);
                return *this;
            }

            template<typename ...Args>
            Class &construct()
            {
                auto constructor = Constructor<ClassT, Args...>::Construct;
                m_class_reg->AddIndex(
                    std::make_shared<FunctionRegistry>(
                        new Function<decltype(constructor)>("__init__", constructor)
                    )
                );

                return *this;
            };

            template<typename MemberFunction>
            Class &def(const std::string &name, MemberFunction f)
            {
                m_class_reg->AddIndex(
                    std::make_shared<FunctionRegistry>(
                        new Function<MemberFunction>(name, f)
                    )
                );
                return *this;
            }

            template<typename T>
            Class &def_read(const std::string &name, T ClassT:: *member)
            {
                std::function<T &(ClassT &)> getter = Getter<T>(member);
                m_class_reg->AddIndex(
                    std::make_shared<PropertyRegistry>(
                        new Function<decltype(getter)>(name, getter)
                    )
                );

                return *this;
            }

            template<typename T>
            Class &def_write(const std::string &name, T ClassT:: *member)
            {
                std::function<void (ClassT &, const T &)> setter = Setter<T>(member);
                m_class_reg->AddNewIndex(
                    std::make_shared<PropertyRegistry>(
                        new Function<decltype(setter)>(name, setter)
                    )
                );

                return *this;
            }

            template<typename T>
            Class &def_rdwr(const std::string &name, T ClassT:: *member)
            {
                def_read(name, member);
                def_write(name, member);
                return *this;
            }

        private:
            template<typename T>
            struct ClassCast
            {
                static void *call(void *p)
                {
                    return static_cast<T *>(static_cast<ClassT *>(p));
                }
            };

            template<typename T>
            class Getter
            {
                public:
                    Getter(T ClassT:: *member):
                        m_member(member) { }
                    ~Getter() = default;

                    T &operator () (ClassT &obj)
                    {
                        return obj.*m_member;
                    }

                private:
                    T ClassT:: *m_member;
            };

            template<typename T>
            class Setter
            {
                public:
                    Setter(T ClassT:: *member):
                        m_member(member) { }
                    ~Setter() = default;

                    void operator () (ClassT &obj, const T &value)
                    {
                        obj.*m_member = value;
                    }

                private:
                    T ClassT:: *m_member;
            };

            void AddBaseList(type_list<> /*empty*/) { }

            template<typename Base0, typename ...BaseN>
            void AddBaseList(type_list<Base0, BaseN...> /*base_list*/)
            {
                m_class_reg->AddBase(typeid(Base0).hash_code(), ClassCast<Base0>::call);
                AddBaseList(type_list<BaseN...>());
            }

            std::shared_ptr<ClassRegistry> m_class_reg;
    };

} }


namespace Luapp {

    template<typename ClassT, typename ...Bases>
    detail::Class<ClassT, Bases...> class_(const std::string &name)
    {
        return detail::Class<ClassT, Bases...>(name);
    }

}

#endif
