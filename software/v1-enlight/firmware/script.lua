-- Wash Firmware

setup = function()
    currentMode = 0
    version = "0.1"
    printMessage("Kanatnikoff Wash v." .. version)
    printMessage("programs stop:" .. program.stop)
    return 0
end

-- loop is being executed
loop = function()
    welcome:Display()
    turn_light(1, animation.one_button)
    -- key = get_key()
    --if get_key() == 1 then
    --    currentMode =  1 - currentMode
    --    printMessage("currentMode: " .. currentMode)
    --end
--    if(currentMode == 1) then
    --    run_program(soap)
--    else
    --    run_program(stop)
--    end
    smart_delay(100)
    return 0
end

get_key = function()
    return hardware:GetKey()
end

smart_delay = function(ms)
    hardware:SmartDelay(ms)
end

turn_light = function(rel_num, animation_code)
    hardware:TurnLight(rel_num, animation_code)
end

run_program = function(program_num)
    hardware:TurnProgram(program_num)
end
