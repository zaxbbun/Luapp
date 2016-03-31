EventMgr = { ev_map = { } }

function EventMgr:RegisterEvent(ev_type, cb)
    local ev_map = self.ev_map
    ev_map[ev_type] = ev_map[ev_type] or { }

    local ev_list = self.ev_map[ev_type]
    local ev_id = #ev_list + 1
    ev_list[ev_id] = cb

    return function ()
        ev_list[ev_id] = nil
        if #ev_list == 0 then
            self.ev_map[ev_type] = nil
        end
    end
end

function EventMgr:Trigger(ev_type, ...)
    local ev_list = self.ev_map[ev_type]
    if not ev_list then
        return
    end

    for _, cb in pairs(ev_list) do
        cb(...)
    end
end

function Trigger(ev_type, ...)
    EventMgr:Trigger(ev_type, ...)
end
