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

#include <sstream>
#include "class_meta.hpp"

namespace luapp { namespace detail {

    int PropertyTag(lua_State *lua);

    static int DispatchConstructor(lua_State *lua)
    {
        int argc = lua_gettop(lua);
        const ClassMeta &class_meta = *static_cast<const ClassMeta *>(lua_touserdata(lua, 1));

        class_meta.PushIndexTable();
        lua_pushstring(lua, "__init__");
        lua_rawget(lua, -2);
        lua_insert(lua, 1);
        lua_pop(lua, 1);

        if(lua_pcall(lua, argc, 1, 0)){
            lua_error(lua);
        }

        return 1;
    }

    static int DispatchIndex(const ClassMeta &class_meta)
    {
        lua_State *lua = class_meta.Lua();

        if(!class_meta.FindIndex(2)){
            lua_pushnil(lua);
            return 1;
        }

        if(lua_tocfunction(lua, -1) == PropertyTag){
            lua_getupvalue(lua, -1, 1);
            lua_pushvalue(lua, 1);
            if(lua_pcall(lua, 1, 1, 0)){
                lua_error(lua);
            }
        }

        return 1;
    }

    static int DispatchNewIndex(const ClassMeta &class_meta)
    {
        lua_State *lua = class_meta.Lua();

        if(!class_meta.FindNewIndex(2)){
            lua_pushnil(lua);
            return 0;
        }

        if(lua_tocfunction(lua, -1) == PropertyTag){
            lua_getupvalue(lua, -1, 1);
            lua_pushvalue(lua, 1);
            lua_pushvalue(lua, 3);
            if(lua_pcall(lua, 2, 0, 0)){
                lua_error(lua);
            }
        }

        return 0;
    }

    static int DispatchClassIndex(lua_State *lua)
    {
        ClassMeta *class_meta = static_cast<ClassMeta *>(lua_touserdata(lua, 1));
        return DispatchIndex(*class_meta);
    }

    static int DispatchClassNewIndex(lua_State *lua)
    {
        ClassMeta *class_meta = static_cast<ClassMeta *>(lua_touserdata(lua, 1));
        return DispatchNewIndex(*class_meta);
    }

    static int DispatchObjectIndex(lua_State *lua)
    {
        ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, 1));
        return DispatchIndex(obj_meta->GetClassMeta());
    }

    static int DispatchObjectNewIndex(lua_State *lua)
    {
        ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, 1));
        return DispatchNewIndex(obj_meta->GetClassMeta());
    }

    static int DestructClassMetaMgr(lua_State *lua)
    {
        ClassMetaMgr *class_meta = static_cast<ClassMetaMgr *>(lua_touserdata(lua, 1));
        class_meta->~ClassMetaMgr();
        return 0;
    }

    static int DestructClassMeta(lua_State *lua)
    {
        ClassMeta *class_meta = static_cast<ClassMeta *>(lua_touserdata(lua, 1));
        class_meta->~ClassMeta();
        return 0;
    }

    static int DestructObjectMeta(lua_State *lua)
    {
        ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, 1));
        obj_meta->~ObjectMeta();
        return 0;
    }

    static int GetClassMetaDoc(lua_State *lua)
    {
        ClassMeta *class_meta = static_cast<ClassMeta *>(lua_touserdata(lua, -1));
        bool error = false;

        try{
            std::ostringstream doc;
            doc << class_meta->Document();
            lua_pushstring(lua, doc.str().c_str());
        }
        catch(const std::runtime_error &e){
            lua_pushstring(lua, e.what());
            error = true;
        }

        if(error){
            lua_error(lua);
        }

        return 1;
    }

    static int GetObjectMetaDoc(lua_State *lua)
    {
        ObjectMeta *obj_meta = static_cast<ObjectMeta *>(lua_touserdata(lua, 1));
        lua_pushlightuserdata(lua, (void *)(&obj_meta->GetClassMeta()));
        return GetClassMetaDoc(lua);
    }

    static void PushClassMgrMetatable(lua_State *lua)
    {
        lua_pushstring(lua, "__luapp_class_mgr_metatable");
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
        lua_pushcfunction(lua, DestructClassMetaMgr);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__luapp_class_mgr_metatable");
        lua_pushvalue(lua, -2);
        lua_rawset(lua, LUA_REGISTRYINDEX);
    }

    void PushClassMetatable(lua_State *lua)
    {
        lua_pushstring(lua, "__luapp_class_metatable");
        lua_rawget(lua, LUA_REGISTRYINDEX);

        if(lua_istable(lua, -1)){
            return;
        }

        lua_pop(lua, 1);
        lua_newtable(lua);

        lua_pushstring(lua, "__metatable");
        lua_pushstring(lua, "protected metatable: readonly");
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__call");
        lua_pushcfunction(lua, DispatchConstructor);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__gc");
        lua_pushcfunction(lua, DestructClassMeta);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, DispatchClassIndex);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__newindex");
        lua_pushcfunction(lua, DispatchClassNewIndex);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__tostring");
        lua_pushcfunction(lua, GetClassMetaDoc);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__luapp_class_metatable");
        lua_pushvalue(lua, -2);
        lua_rawset(lua, LUA_REGISTRYINDEX);
    }

    static void PushObjectMetatable(lua_State *lua)
    {
        lua_pushstring(lua, "__luapp_object_metatable");
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
        lua_pushcfunction(lua, DestructObjectMeta);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, DispatchObjectIndex);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__newindex");
        lua_pushcfunction(lua, DispatchObjectNewIndex);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__tostring");
        lua_pushcfunction(lua, GetObjectMetaDoc);
        lua_rawset(lua, -3);

        lua_pushstring(lua, "__luapp_object_metatable");
        lua_pushvalue(lua, -2);
        lua_rawset(lua, LUA_REGISTRYINDEX);
    }

    ClassMeta::ClassMeta(lua_State *lua, const std::string &name, size_t type_hash):
        name_(name),
        type_hash_(type_hash)
    {
        index_table_    = make_table(lua);
        newindex_table_ = make_table(lua);
        GetClassMetaMgr(lua).AddClassMeta(this);
    }

    Object ClassMeta::Document() const
    {
        lua_State *lua = index_table_.Lua();
        Object doc = make_table(lua);

        doc["__index"] = index_table_;
        doc["__newindex"] = newindex_table_;

        Object base_list = make_table(lua);
        doc["__baselist"] = base_list;

        for(const BaseClass &base : base_list_){
            const ClassMeta &meta = base.meta;
            base_list[meta.name_] = meta.Document();
        }

        return doc;
    }

    bool ClassMeta::FindIndex(int key) const
    {
        lua_State *lua = Lua();

        PushIndexTable();
        lua_pushvalue(lua, key);
        lua_rawget(lua, -2);

        if(!lua_isnil(lua, -1)){
            return true;
        }

        lua_pop(lua, 2);
        for(const BaseClass &base : base_list_){
            if(base.meta.FindIndex(key)){
                return true;
            }
        }

        return false;
    }

    bool ClassMeta::FindNewIndex(int key) const
    {
        lua_State *lua = Lua();

        PushNewIndexTable();
        lua_pushvalue(lua, key);
        lua_rawget(lua, -2);

        if(!lua_isnil(lua, -1)){
            return true;
        }

        lua_pop(lua, 2);
        for(const BaseClass &base : base_list_){
            if(base.meta.FindNewIndex(key)){
                return true;
            }
        }

        return false;
    }

    bool ClassMeta::Convertable(size_t type_hash) const
    {
        if(type_hash_ == type_hash){
            return true;
        }

        for(const BaseClass &base : base_list_){
            if(base.meta.Convertable(type_hash)){
                return true;
            }
        }

        return false;
    }

    void *ClassMeta::Cast(void *ptr, size_t type_hash) const
    {
        if(type_hash_ == type_hash){
            return ptr;
        }

        for(const BaseClass &base : base_list_){
            void *casted_ptr = base.meta.Cast(base.cast(ptr), type_hash);
            if(casted_ptr){
                return casted_ptr;
            }
        }

        return nullptr;
    }

    void ClassMetaMgr::AddClassMeta(const ClassMeta *class_meta)
    {
        class_map_[class_meta->TypeHash()] = class_meta;
    }

    const ClassMeta *ClassMetaMgr::GetClassMeta(size_t type_hash) const
    {
        auto iter = class_map_.find(type_hash);
        return iter != class_map_.end() ? iter->second : nullptr;
    }

    ClassMetaMgr &GetClassMetaMgr(lua_State *lua)
    {
        static int __luapp_class_mgr_instance = 0;

        lua_pushlightuserdata(lua, &__luapp_class_mgr_instance);
        lua_rawget(lua, LUA_REGISTRYINDEX);

        if(lua_isuserdata(lua, -1)){
            ClassMetaMgr *mgr = static_cast<ClassMetaMgr *>(lua_touserdata(lua, -1));
            lua_pop(lua, 1);
            return *mgr;
        }

        ClassMetaMgr *mgr = static_cast<ClassMetaMgr *>(lua_newuserdata(lua, sizeof(*mgr)));
        new (mgr) ClassMetaMgr();

        PushClassMgrMetatable(lua);
        lua_setmetatable(lua, -2);

        lua_pushlightuserdata(lua, &__luapp_class_mgr_instance);
        lua_pushvalue(lua, -2);
        lua_rawset(lua, LUA_REGISTRYINDEX);

        lua_pop(lua, 2);
        return *mgr;
    }

    void CreateObjectMeta(const ClassMeta &class_meta, IObjectHolder *holder)
    {
        lua_State *lua = class_meta.Lua();

        ObjectMeta *obj_meta = (ObjectMeta *)(lua_newuserdata(lua, sizeof(ObjectMeta)));
        new (obj_meta) ObjectMeta(class_meta, holder);

        PushObjectMetatable(lua);
        lua_setmetatable(lua, -2);
    }

    void CreateObjectMeta(lua_State *lua, size_t type_hash, const std::string &type_name,
        IObjectHolder *holder)
    {
        const ClassMeta *class_meta = GetClassMetaMgr(lua).GetClassMeta(type_hash);
        if(!class_meta){
            luaL_error(lua, "cannot pass value of unregistered type<%s>!", type_name.c_str());
            return;
        }

        CreateObjectMeta(*class_meta, holder);
    };

} }
