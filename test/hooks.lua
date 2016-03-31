local EVENT = require 'event'

local function OnCreatureBorn(creature)
    print(creature:Name() .. ' borned')
end

local function OnPlayerLogin(player)
    print(player:Name() .. ' login')
    player.honor = 70
    print(player:Name() .. ' honor: ' .. player.honor)
end

local function OnPlayerLogout(player)
    print(player:Name() .. ' logout')
end

local function OnPlayerKill(player, creature)
    print(player:Name() .. ' killed ' .. creature:Name() .. ', add 10 honor')
    player.honor = player.honor + 10
    print(player:Name() .. ' honor: ' .. player.honor)
end

local function OnCreatureHurt(creature)
    print(creature:Name() .. ' been hurt, curr hp:' .. creature:GetHP())
end

EVENT.register(EventType.kCreatureBorn, OnCreatureBorn)
EVENT.register(EventType.kPlayerLogin, OnPlayerLogin)
EVENT.register(EventType.kPlayerLogout, OnPlayerLogout)
EVENT.register(EventType.kPlayerKill, OnPlayerKill)
EVENT.register(EventType.kCreatureHurt, OnCreatureHurt)
