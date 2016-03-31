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

#include <fstream>
#include <sstream>

#include "aes.hpp"
#include "luapp.hpp"

namespace luapp {

    static void *DefaultAllocator(void *ud, void *p, size_t osz, size_t nsz)
    {
        size_t &used = *(size_t *)ud;

        if(nsz == 0){
            used -= osz;
            free(p);
            return nullptr;
        }

        void *q = realloc(p, nsz);
        if(q){
            if(p){
                used += nsz - osz;
            }
            else{
                used += nsz;
            }
        }

        return q;
    }

    static int DefaultScriptSearcher(lua_State *lua)
    {
        const uint32_t kMagic = 0x8716a700;

        Object package = global(lua)["package"];
        Object script_path = package["path"];
        Object search_path = package["searchpath"];

        int top = lua_gettop(lua);

        search_path.Push();
        lua_pushvalue(lua, 1);
        lua_pushstring(lua, (const char *)script_path);

        lua_pcall(lua, 2, LUA_MULTRET, 0);
        if(lua_gettop(lua) - top == 2){
            return 1;
        }

        std::string filename = lua_tostring(lua, -1);
        std::ifstream script(filename.c_str(), std::ios::binary);
        if(!script){
            return 1;
        }

        script.seekg(0, std::ios::end);
        size_t buff_size = script.tellg();
        script.seekg(0, std::ios::beg);

        char *buff = new char[buff_size];
        script.read(buff, buff_size);

        uint32_t magic = *(uint32_t *)(buff);
        uint32_t size  = *(uint32_t *)(buff + sizeof(magic));

        if(magic != kMagic){
            delete[] buff;
            luaL_error(lua, "failed to load module %s -- %s", filename.c_str(),
                "invalid file type for DefaultScriptSearcher!");
        }

        Object aes = registry(lua)["__luapp_aes_crypt"];
        aes.Push();

        aes_crypt *crypt = static_cast<aes_crypt *>(lua_touserdata(lua, -1));
        char *chunk = buff + sizeof(magic) + sizeof(size);
        aes_decrypt(crypt, chunk, size, chunk);
        lua_pop(lua, 1);

        int code = luaL_loadbuffer(lua, chunk, size, 0);
        delete[] buff;

        if(code){
            luaL_error(lua, "failed to load module %s -- %s", filename.c_str(),
                lua_tostring(lua, -1));
        }

        lua_pushstring(lua, filename.c_str());
        return 2;
    }

    LuaState::LuaState(bool openlibs):
        used_(0),
        lua_(lua_newstate(&DefaultAllocator, &used_)),
        owner_(true),
        global_(global(lua_))
    {
        luaL_checkversion(lua_);

        if(openlibs){
            luaL_openlibs(lua_);
        }
    }

    LuaState::LuaState(lua_State *lua, bool openlibs):
        used_(0),
        lua_(lua),
        owner_(false),
        global_(global(lua_))
    {
        luaL_checkversion(lua_);

        if(openlibs){
            luaL_openlibs(lua_);
        }
    }

    LuaState::~LuaState()
    {
        Object crypt = registry(lua_)["__luapp_aes_crypt"];
        if(crypt.Valid()){
            crypt.Push();
            aes_crypt *aes = static_cast<aes_crypt *>(lua_touserdata(lua_, -1));
            lua_pop(lua_, 1);

            aes_destroy(aes);
            registry(lua_)["__luapp_aes_crypt"] = nullptr;
        }

        crypt.Release();
        global_.Release();

        if(owner_){
            lua_close(lua_);
        }
    }

    int LuaState::GarbageCollect(int what, int data)
    {
        return lua_gc(lua_, what, data);
    }

    void LuaState::RegisterSearcher(lua_CFunction searcher)
    {
        Object searchers = Global("package")["searchers"];
        searchers.Push();

        const int kLuaSearcherIndex = 2;
        lua_pushcfunction(lua_, searcher);
        lua_rawseti(lua_, -2, kLuaSearcherIndex);

        lua_pop(lua_, 1);
    }

    bool LuaState::RegisterDefaultSearcher(const char *key)
    {
        Object crypt = registry(lua_)["__luapp_aes_crypt"];
        if(crypt.IsNil()){
            aes_crypt *aes = aes_create(key);
            if(!aes){
                fprintf(stderr, "[luapp] RegisterDefaultSearcher failed, "
                    "key length must be 16/24/32 bytes long!\n");
                return false;
            }
            registry(lua_)["__luapp_aes_crypt"] = (void *)aes;
        }

        RegisterSearcher(DefaultScriptSearcher);
        return true;
    }

    void LuaState::AddScriptPath(const char *path)
    {
        std::ostringstream search_path;
        search_path << "package.path = package.path .. ';" << path << "'";
        LoadString(search_path.str());
    }

    void LuaState::AddLibraryPath(const char *path)
    {
        std::ostringstream search_path;
        search_path << "package.cpath = package.cpath .. ';" << path << "'";
        LoadString(search_path.str());
    }

    bool LuaState::LoadFile(const char *filename, bool reload)
    {
        if(reload){
            Global("package")["loaded"][filename] = nullptr;
        }

        Global("require").call(filename);
        return true;
    }

    bool LuaState::LoadBuff(const char *buff, uint32_t size)
    {
        lua_pushcfunction(lua_, detail::Traceback);

        if(luaL_loadbuffer(lua_, buff, size, "") || lua_pcall(lua_, 0, 0, -2)){
            fprintf(stderr, "\n[luapp]: %s\n", lua_tostring(lua_, -1));
            lua_pop(lua_, 2);
            return false;
        }

        lua_pop(lua_, 1);
        return true;
    }

    bool LuaState::LoadString(const std::string &buff)
    {
        lua_pushcfunction(lua_, detail::Traceback);

        if(luaL_loadstring(lua_, buff.c_str()) || lua_pcall(lua_, 0, 0, -2)){
            fprintf(stderr, "\n[luapp]: %s\n", lua_tostring(lua_, -1));
            lua_pop(lua_, 2);
            return false;
        }

        lua_pop(lua_, 1);
        return true;
    }

    Object LuaState::Eval(const std::string &buff)
    {
        detail::StackPop pop(lua_, 1);

        if(luaL_loadstring(lua_, buff.c_str())){
            throw std::runtime_error(lua_tostring(lua_, -1));
        }

        return Object(lua_, 1);
    }

}
