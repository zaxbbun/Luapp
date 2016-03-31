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

#include "Luapp.hpp"
#include "example.hpp"
#include <iostream>


extern "C" int luaopen_example(lua_State *lua);


void TestLuapp(Luapp::LuaState &lua)
{
    Luapp::Object trigger = lua.Global("Trigger");

    Creature creature("leo", 40, 40, 100);
    trigger.call(EventType::kCreatureBorn, creature);

    Player player("jax", 40, 70, 100, 10, 20);
    trigger.call(EventType::kPlayerLogin, player);

    while(creature.GetHP() > 0){
        player.Attack(creature);
        player.SpellHit(creature);
        trigger.call(EventType::kCreatureHurt, creature);
    }

    trigger.call(EventType::kPlayerKill, player, creature);
    trigger.call(EventType::kPlayerLogout, player);
}


int main()
{
    Luapp::LuaState lua;

    lua.LoadString(R"(
        function sum(a, b)
            return a + b
        end

        function get()
            return { name = 'get' }
        end

        function set(t)
            t.name = 'set'
            return t
        end
    )");

    std::cout << "sum(4, 7) = " << lua.Global("sum").call<int>(4, 7) << std::endl;

    const Luapp::Object &r = lua.Global("get").call<Luapp::Object>();
    std::cout << r;

    Luapp::Object t = Luapp::make_table(lua.Lua());
    lua.Global("set").call<Luapp::Object>(t);
    std::cout << t;

    t["self"] = t;
    std::cout << t;

    // register searcher for encrypted lua scripts.
    // lua.RegisterDefaultSearcher("just4codea27f827");
    lua.AddScriptPath("../test/?.lua");
    luaopen_example(lua.Lua());

    lua.LoadFile("Hooks");
    lua.Global("RegisterHooks").call();

    TestLuapp(lua);
    return 0;
}
