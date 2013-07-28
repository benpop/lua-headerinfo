--
-- Place Lua function dump header information into a table
--
-- Author: benpop
-- Date: 2013-07-27
--

function headerinfo (f)
  f = f or function () end
  if type(f) == "function" then
    local flag, val = pcall(string.dump, f)
    if flag then
      f = val
    else
      return nil, val  -- val is error message
    end
  elseif type(f) == "string" then
    if string.sub(f, 1, 4) ~= "\27Lua" then
      return nil, "not a valid Lua function dump"
    end
  else
    return nil, "not a function or function dump"
  end
  local version, format, endian_flag,
        int_size, size_size, instruction_size, number_size,
        number_is_int_flag = f:byte(5, 12)
  local major, minor = math.floor(version / 16), version % 16
  return {
    version = string.format("%d.%d", major, minor),
    format = format,
    endianness = endian_flag ~= 0 and "little-endian" or "big-endian",
    int_size = int_size,
    size_size = size_size,
    instruction_size = instruction_size,
    number_size = number_size,
    number_is_int = number_is_int_flag ~= 0
  }
end

return headerinfo
