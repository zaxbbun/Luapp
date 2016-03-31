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

#ifndef _WIN32
#include <cxxabi.h>
#endif
#include <sstream>

#include "function.hpp"

namespace luapp { namespace detail {

    static void PushFunctionMetatable(lua_State *lua)
    {
        lua_pushstring(lua, "__luapp_function_metatable");
        lua_rawget(lua, LUA_REGISTRYINDEX);

        if(lua_istable(lua, -1)){
            return;
        }

        lua_pop(lua, 1);
        lua_newtable(lua);

        lua_pushstring(lua, "__metatable");
        lua_pushstring(lua, "protected metatable: readonly");
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__gc");
        lua_pushcfunction(lua, FunctionBase::DestroyFunction);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__luapp_function_metatable");
        lua_pushvalue(lua, -2);
        lua_rawset(lua, LUA_REGISTRYINDEX);
    }

    void FunctionBase::RegisterFunction(lua_State *lua, FunctionBase *func)
    {
        lua_pushstring(lua, func->name_.c_str());
        lua_rawget(lua, -2);

        if(lua_isnil(lua, -1)){
            lua_pop(lua, 1);
            lua_pushstring(lua, func->name_.c_str());
            CreateFunction(lua, func);
            lua_rawset(lua, -3);
            return;
        }

        lua_getupvalue(lua, -1, 1);

        FunctionBase *sibling = *(FunctionBase **)lua_touserdata(lua, -1);
        func->next_ = sibling->next_;
        sibling->next_ = func;

        lua_pop(lua, 2);
    }

    void FunctionBase::CreateFunction(lua_State *lua, FunctionBase *func)
    {
        *(FunctionBase **)lua_newuserdata(lua, sizeof(FunctionBase *)) = func;
        PushFunctionMetatable(lua);
        lua_setmetatable(lua, -2);

        lua_pushcclosure(lua, &FunctionBase::Call, 1);
    }

    int FunctionBase::DestroyFunction(lua_State *lua)
    {
        FunctionBase *func = *(FunctionBase **)lua_touserdata(lua, -1);
        delete func;
        return 0;
    }

    int FunctionBase::Call(lua_State *lua)
    {
        FunctionBase *func = *(FunctionBase **)lua_touserdata(lua, lua_upvalueindex(1));
        OverloadsFilter filter = { 0, 0, { 0 } };

        bool error = false;
        int nresults;

        do{
            try{
                nresults = (*func)(lua, filter);
            }
            catch(const std::exception &e){
                lua_pushstring(lua, e.what());
                error = true;
                break;
            }

            if(!filter.find){
                std::ostringstream info;
                info<< "=====================================" << std::endl
                    << "[luapp] invalid function! candidates:" << std::endl
                    << "=====================================" << std::endl;

                for(int i = 0; i < filter.index; ++i){
                    info << "(" << i << ") " << filter.overloads[i]->Signature() << std::endl;
                }

                lua_pushstring(lua, info.str().c_str());
                error = true;
            }
        }while(0);

        if(error){
            lua_error(lua);
        }

        return nresults;
    }

    FunctionBase::FunctionBase(const std::string &name, const std::string &signature):
        name_(name),
#ifdef _WIN32
        signature_(signature),
#endif
        next_(nullptr)
    {
#ifndef _WIN32
        char buff[1024];
        size_t length = sizeof(buff);
        signature_ = abi::__cxa_demangle(signature.c_str(), buff, &length, 0);
#endif
    }

} }
