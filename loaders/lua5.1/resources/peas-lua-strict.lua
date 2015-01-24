--
--  Copyright (C) 2015 - Garrett Regier
--
-- libpeas is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser General Public
-- License as published by the Free Software Foundation; either
-- version 2.1 of the License, or (at your option) any later version.
--
-- libpeas is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public
-- License along with this library; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
--
-- Modified version of: http://metalua.luaforge.net/src/lib/strict.lua.html

__STRICT = true

local mt = getmetatable(_G)
if mt == nil then
    mt = {}
    setmetatable(_G, mt)
end

function mt:__newindex(name, value)
    if __STRICT then
        local what = debug.getinfo(2, 'S').what

        if what ~= 'C' then
            error("Attempted to create global variable '" ..
                  tostring(name) .. "'", 2)
        end
    end

    rawset(self, name, value)
end

function mt:__index(name)
    if not __STRICT or debug.getinfo(2, 'S').what == 'C' then
        return rawget(self, name)
    end

    error("Attempted to access nonexistent " ..
          "global variable '" .. tostring(name) .. "'", 2)
end

-- ex:ts=4:et:
