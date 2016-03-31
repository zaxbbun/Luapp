#! /usr/bin/env lua

require 'example'
require 'Hooks'


local function main()
    RegisterHooks()

    local creature = Creature("leo", 40, 40, 100);
    Trigger(EventType.kCreatureBorn, creature);

    local player = Player("jax", 40, 70, 100, 10, 20);
    Trigger(EventType.kPlayerLogin, player);

    while creature:GetHP() > 0 do
        player:Attack(creature);
        player:SpellHit(creature);
        Trigger(EventType.kCreatureHurt, creature);
    end

    Trigger(EventType.kPlayerKill, player, creature);
    Trigger(EventType.kPlayerLogout, player);
end


local ok, STP = pcall(require, 'StackTracePlus')
local traceback = ok and STP.stacktrace or debug.traceback

xpcall(main, function(...) print('\n' .. traceback(...)) end)
