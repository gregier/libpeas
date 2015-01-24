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

local io = require 'io'
local os = require 'os'


local function check(err, format, ...)
    if err == nil then
        return
    end

    io.stderr:write(('Error: %s:\n%s\n'):format(format:format(...), err))
    os.exit(1)
end


local function main(arg)
    for i = 1, #arg, 2 do
        local filename = arg[i]
        local output = arg[i + 1]

        local input_file, err = io.open(filename, 'rb')
        check(err, 'Failed to open file "%s"', filename)

         -- Error includes the filename
        local compiled, err = loadstring(input_file:read('*a'),
                                         '@' .. filename)
        check(err, 'Invalid Lua file')

        local f, err = io.open(output, 'wb')
        check(err, 'Failed to open file "%s"', output)

        local success, err = f:write(string.dump(compiled))
        check(err, 'Failed to write to "%s"', output)

        local success, err = f:close()
        check(err, 'Failed to save "%s"', output)
    end
end


os.exit(main(arg) or 0)

-- ex:ts=4:et:
