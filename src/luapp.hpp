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

#ifndef LUAPP_HPP
#define LUAPP_HPP

#include "object.hpp"
#include "module.hpp"
#include "enum.hpp"
#include "class.hpp"
#include "function.hpp"

namespace luapp {

    class LuaState
    {
        public:
            LuaState(bool openlibs = true);
            LuaState(lua_State *lua, bool openlibs = false);
            ~LuaState();

            lua_State *Lua()
            {
                return lua_;
            }

            Object &Global()
            {
                return global_;
            }

            Object Global(const char *name)
            {
                return global_[name];
            }

            size_t MemoryUsage() const
            {
                return used_;
            }

            int GarbageCollect(int what, int data);

            void RegisterSearcher(lua_CFunction searcher);
            bool RegisterDefaultSearcher(const char *key);

            void AddScriptPath(const char *path);
            void AddLibraryPath(const char *path);

            bool LoadFile(const char *filename, bool reload = false);
            bool LoadBuff(const char *buff, uint32_t size);
            bool LoadString(const std::string &buff);

            Object Eval(const std::string &buff);

        private:
            size_t used_;
            lua_State *lua_;
            bool owner_;
            Object global_;
    };

}

#endif
