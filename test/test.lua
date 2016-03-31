#! /usr/bin/env lua

require 'example'
require 'hooks'

local EVENT = require 'event'

local function main()
    local trigger = EVENT.trigger

    local npc = Creature("leo", 40, 40, 100);
    trigger(EventType.kCreatureBorn, npc);

    local player = Player("jax", 40, 70, 100, 10, 20);
    trigger(EventType.kPlayerLogin, player);

    while npc:GetHP() > 0 do
        player:Attack(npc);
        player:SpellHit(npc);
        trigger(EventType.kCreatureHurt, npc);
    end

    trigger(EventType.kPlayerKill, player, npc);
    trigger(EventType.kPlayerLogout, player);
end

local ok, STP = pcall(require, 'StackTracePlus')
local traceback = ok and STP.stacktrace or debug.traceback

xpcall(main, function(...) print('\n' .. traceback(...)) end)
