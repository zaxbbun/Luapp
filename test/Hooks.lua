require 'EventMgr'

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

function RegisterHooks()
    EventMgr:RegisterEvent(EventType.kCreatureBorn, OnCreatureBorn)
    EventMgr:RegisterEvent(EventType.kPlayerLogin, OnPlayerLogin)
    EventMgr:RegisterEvent(EventType.kPlayerLogout, OnPlayerLogout)
    EventMgr:RegisterEvent(EventType.kPlayerKill, OnPlayerKill)
    EventMgr:RegisterEvent(EventType.kCreatureHurt, OnCreatureHurt)
end
