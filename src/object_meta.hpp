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

#ifndef LUAPP_OBJECT_META_HPP
#define LUAPP_OBJECT_META_HPP

#include <cstddef>
#include <type_traits>

namespace luapp { namespace detail {

    class IObjectHolder
    {
        public:
            IObjectHolder() = default;
            virtual ~IObjectHolder() = default;

            virtual bool IsConst() = 0;
            virtual void *GetObject() = 0;
    };

    template<typename T>
    class ValueHolder: public IObjectHolder
    {
        public:
            ValueHolder(const T &value):
                obj_(value) { }
            ~ValueHolder() = default;

            bool IsConst() override
            {
                return std::is_const<T>::value;
            }

            void *GetObject() override
            {
                return static_cast<void *>(&obj_);
            }

        private:
            T obj_;
    };

    template<typename T>
    class PointerHolder: public IObjectHolder
    {
        public:
            PointerHolder(T p, bool gc):
                obj_(p),
                gc_(gc) { }

            ~PointerHolder()
            {
                if(gc_){
                    delete obj_;
                }
            }

            bool IsConst() override
            {
                return std::is_const<typename std::remove_pointer<T>::type>::value;
            }

            void *GetObject() override
            {
                return const_cast<
                    typename std::remove_cv<typename std::remove_pointer<T>::type>::type *
                >(obj_);
            }

        private:
            T obj_;
            bool gc_;
    };

    class ClassMeta;

    class ObjectMeta
    {
        public:
            ObjectMeta(const ClassMeta &class_meta, IObjectHolder *holder):
                class_meta_(class_meta),
                holder_(holder)
            {

            }

            ~ObjectMeta()
            {
                delete holder_;
            }

            const ClassMeta &GetClassMeta() const
            {
                return class_meta_;
            }

            bool Match(size_t type_hash, bool is_const) const;
            void *GetObject(size_t target_type);

        private:
            const ClassMeta &class_meta_;
            IObjectHolder *holder_;
    };

} }

#endif
