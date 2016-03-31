local uid    = 0
local events = { }

local EVENT = { }

function EVENT.register(type, cb)
    local subset = events[type]
    if subset == nil then
        subset = { }
        events[type] = subset
    end

    uid = uid + 1
    subset[uid] = cb

    return function ()
        subset[uid] = nil
    end
end

function EVENT.trigger(type, ...)
    local subset = events[type]
    if subset == nil then return end

    for _, cb in pairs(subset) do
        cb(...)
    end
end

return EVENT
