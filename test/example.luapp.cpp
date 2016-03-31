/* 
 * This file was generated by the luagen compiler. DO NOT EDIT!
 * 
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

#include <Luapp.hpp>

#include "example.hpp"


extern "C" int luaopen_example(lua_State *lua)
{
    using namespace Luapp;

    module(lua)[
        enum_("EventType")
            .def("kCreatureBorn", 0)
            .def("kPlayerLogin", 1)
            .def("kPlayerLogout", 2)
            .def("kPlayerKill", 3)
            .def("kCreatureHurt", 4),
        class_<Creature>("Creature")
            .construct<const char *, int, int, int>()
            .def("Name", (std::string (Creature::*)() const) &Creature::Name)
            .def("GetX", (int (Creature::*)() const) &Creature::GetX)
            .def("GetY", (int (Creature::*)() const) &Creature::GetY)
            .def("GetHP", (int (Creature::*)() const) &Creature::GetHP)
            .def("Damage", (void (Creature::*)(int)) &Creature::Damage),
        class_<Player, Creature>("Player")
            .construct<const char *, int, int, int, int, int>()
            .def("Attack", (void (Player::*)(Creature &)) &Player::Attack)
            .def("SpellHit", (void (Player::*)(Creature &)) &Player::SpellHit)
            .def_rdwr("honor", &Player::honor)
    ];

    return 0;
}
