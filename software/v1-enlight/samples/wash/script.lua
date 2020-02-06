-- Wash Firmware

setup = function()
    -- global variables
    balance = 0.0

    min_electron_balance = 100
    max_electron_balance = 900
    electron_amount_step = 25
    electron_balance = min_electron_balance
    
    balance_seconds = 0
    kasse_balance = 0.0
    post_position = 1      

    -- constants
    welcome_mode_seconds = 3
    thanks_mode_seconds = 120
    free_pause_seconds = 120
    wait_card_mode_seconds = 120
    
    hascardreader = false

    price_pause = 0
    price_water = 0
    price_soap = 0
    price_active_soap = 0
    price_osmosian = 0
    price_wax = 0

    init_prices()
    ask_for_money:Set("water.value", price_water)
    ask_for_money:Set("soap.value", price_soap)
    ask_for_money:Set("active_soap.value", price_active_soap)
    ask_for_money:Set("osmosian.value", price_osmosian)
    ask_for_money:Set("wax.value", price_wax)
    ask_for_money:Set("pause.value", price_pause)
    ask_for_money:Set("pause_sec.value", free_pause_seconds)
    
    mode_welcome = 0
    mode_choose_method = 10
    mode_select_price = 20
    mode_wait_for_card = 30
    mode_ask_for_money = 40
    mode_start = 50
    mode_water = 60
    mode_soap = 70
    mode_active_soap = 80
    mode_pause = 90
    mode_osmosian = 100
    mode_wax = 110
    mode_thanks = 120
    
    currentMode = mode_welcome
    version = "0.4"

    printMessage("Kanatnikoff Wash v." .. version)
    return 0
end

init_prices = function()
    price_pause = get_price("price6")
    if price_pause == 0 then price_pause = 18 end

    price_water = get_price("price1")
    if price_water == 0 then price_water = 18 end

    price_soap = get_price("price2")
    if price_soap == 0 then price_soap = 18 end

    price_active_soap = get_price("price3")
    if price_active_soap == 0 then price_active_soap = 36 end

    price_osmosian = get_price("price4")
    if price_osmosian == 0 then price_osmosian = 18 end

    price_wax = get_price("price5")
    if price_wax == 0 then price_wax = 18 end
end

-- loop is being executed
loop = function()
    currentMode = run_mode(currentMode)
    smart_delay(100)
    return 0
end

run_mode = function(new_mode)
    if new_mode == mode_welcome then return welcome_mode() end
    if new_mode == mode_choose_method then return choose_method_mode() end
    if new_mode == mode_select_price then return select_price_mode() end
    if new_mode == mode_wait_for_card then return wait_for_card_mode() end
    if new_mode == mode_ask_for_money then return ask_for_money_mode() end
    if new_mode == mode_start then return start_mode() end
    if new_mode == mode_water then return water_mode() end
    if new_mode == mode_soap then return soap_mode() end
    if new_mode == mode_active_soap then return active_soap_mode() end
    if new_mode == mode_pause then return pause_mode() end
    if new_mode == mode_osmosian then return osmosian_mode() end
    if new_mode == mode_wax then return wax_mode() end
    if new_mode == mode_thanks then return thanks_mode() end
end

welcome_mode = function()
    show_welcome()
    run_stop()
    turn_light(0, animation.idle)
    smart_delay(1000 * welcome_mode_seconds)
    if hascardreader == true then
        return mode_choose_method
    end
    return mode_ask_for_money
end

choose_method_mode = function()
    show_choose_method()
    run_stop()

    -- check animation
    turn_light(0, animation.idle)

    pressed_key = get_key()
    if pressed_key == 5 then
        return mode_select_price
    end
    if pressed_key == 2 then
        return mode_ask_for_money
    end

    return mode_choose_method
end

select_price_mode = function()
    show_select_price(electron_balance)
    run_stop()

    -- check animation
    turn_light(0, animation.idle)

    pressed_key = get_key()
    -- increase amount
    if pressed_key == 1 then
        electron_balance = electron_balance + electron_amount_step
        if electron_balance >= max_electron_balance then
            electron_balance = max_electron_balance
        end
    end
    -- decrease amount
    if pressed_key == 2 then
        electron_balance = electron_balance - electron_amount_step
        if electron_balance <= min_electron_balance then
            electron_balance = min_electron_balance
        end 
    end
    if pressed_key == 6 then
        return mode_wait_for_card
    end

    return mode_select_price
end

wait_for_card_mode = function()
    show_wait_for_card()
    run_stop()

    -- check animation
    turn_light(0, animation.idle)

    waiting_loops = wait_card_mode_seconds * 10;

    request_transaction(electron_balance)
    electron_balance = 0

    while(waiting_loops > 0)
    do
        update_balance()
        if balance > 0.99 then
            status = get_transaction_status()
            if status ~= 0 then 
                -- need to test this
                abort_transaction()
            end
            return mode_start
        end
        smart_delay(100)
        waiting_loops = waiting_loops - 1
    end
    return mode_choose_method
end

ask_for_money_mode = function()
    show_ask_for_money()
    run_stop()
    turn_light(0, animation.idle)
    update_balance()
    if balance > 1.0 then
        return mode_start
    end
    return mode_ask_for_money
end

start_mode = function()
    show_start(balance)
    run_pause()
    turn_light(0, animation.intense)
    balance_seconds = free_pause_seconds    
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    -- user has no reason to start with pause here
    if suggested_mode >= 0 and suggested_mode~=mode_pause then return suggested_mode end
    return mode_start
end

water_mode = function()
    show_water(balance)
    run_water()
    turn_light(1, animation.one_button)
    charge_balance(price_water)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_water
end

soap_mode = function()
    show_soap(balance)
    run_soap()
    turn_light(2, animation.one_button)
    charge_balance(price_soap)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_soap
end

active_soap_mode = function()
    show_active_soap(balance)
    run_active_soap()
    turn_light(3, animation.one_button)
    charge_balance(price_active_soap)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_active_soap
end

pause_mode = function()
    show_pause(balance, balance_seconds)
    run_pause()
    turn_light(6, animation.one_button)
    update_balance()
    if balance_seconds > 0 then
        balance_seconds = balance_seconds - 0.1
    else
        balance_seconds = 0
        charge_balance(price_pause)         
    end
    
    if balance <= 0.01 then return mode_thanks end
    
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_pause
end

osmosian_mode = function()
    show_osmosian(balance)
    run_osmosian()
    turn_light(4, animation.one_button)
    charge_balance(price_osmosian)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_osmosian
end

wax_mode = function()
    show_wax(balance)
    run_wax()
    turn_light(5, animation.one_button)
    charge_balance(price_wax)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_wax
end

thanks_mode = function()
    balance = 0
    show_thanks(thanks_mode_seconds)
    turn_light(1, animation.one_button)
    run_program(program.pause)
    waiting_loops = thanks_mode_seconds * 10;
    
    while(waiting_loops>0)
    do
        show_thanks(waiting_loops/10)
	    pressed_key = get_key()
        if pressed_key > 0 and pressed_key < 7 then
            waiting_loops = 0
        end
        update_balance()
        if balance > 0.99 then return mode_start end
        smart_delay(100)
        waiting_loops = waiting_loops - 1
    end

    send_receipt(post_position, 0, kasse_balance)
    kasse_balance = 0

    return mode_ask_for_money
end


show_welcome = function()
    welcome:Display()
end

show_ask_for_money = function()
    ask_for_money_no_price:Display()
end

show_choose_method = function()
    choose_method:Display()
end

show_select_price = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    select_price:Set("balance.value", balance_int)
    select_price:Display()
end

show_wait_for_card = function()
    wait_for_card:Display()
end

show_start = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    start:Set("balance.value", balance_int)
    start:Display()
end

show_water = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    water:Set("balance.value", balance_int)
    water:Display()
end

show_soap = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    soap:Set("balance.value", balance_int)
    soap:Display()
end

show_active_soap = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    active_soap:Set("balance.value", balance_int)
    active_soap:Display()
end

show_pause = function(balance_rur, balance_sec)
    balance_int = math.ceil(balance_rur)
    sec_int = math.ceil(balance_sec)
    pause:Set("pause_balance.value", sec_int)
    pause:Set("balance.value", balance_int)
    pause:Display()
end

show_osmosian = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    osmosian:Set("balance.value", balance_int)
    osmosian:Display()
end

show_wax = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    wax:Set("balance.value", balance_int)
    wax:Display()
end

show_thanks =  function(seconds_float)
    seconds_int = math.ceil(seconds_float)
    thanks:Set("delay_seconds.value", seconds_int)
    thanks:Display()
end

get_mode_by_pressed_key = function()
    pressed_key = get_key()
    if pressed_key == 1 then return mode_water end
    if pressed_key == 2 then return mode_soap end
    if pressed_key == 3 then return mode_active_soap end
    if pressed_key == 4 then return mode_osmosian end
    if pressed_key == 5 then return mode_wax end
    if pressed_key == 6 then return mode_pause end
    return -1
end

get_key = function()
    return hardware:GetKey()
end

smart_delay = function(ms)
    hardware:SmartDelay(ms)
end

get_price = function(key)
    return registry:ValueInt(key)
end

turn_light = function(rel_num, animation_code)
    hardware:TurnLight(rel_num, animation_code)
end

send_receipt = function(post_pos, is_card, amount)
    hardware:SendReceipt(post_pos, is_card, amount)
end

run_pause = function()
    run_program(program.pause)
end

run_water = function()
    run_program(program.water)
end

run_soap = function()
    run_program(program.soap)
end

run_active_soap = function()
    run_program(program.active_soap)
end

run_osmosian = function()
    run_program(program.osmosian)
end

run_wax = function()
    run_program(program.wax)
end

run_stop = function()
    run_program(program.stop)
end

run_program = function(program_num)
    hardware:TurnProgram(program_num)
end

request_transaction = function(money)
    return hardware:RequestTransaction(money)
end

get_transaction_status = function()
    return hardware:GetTransactionStatus()
end

abort_transaction = function()
    return hardware:AbortTransaction()
end

update_balance = function()
    new_coins = hardware:GetCoins()
    new_banknotes = hardware:GetBanknotes()
    new_electronical = hardware:GetElectronical()

    kasse_balance = kasse_balance + new_coins
    kasse_balance = kasse_balance + new_banknotes
    kasse_balance = kasse_balance + new_electronical
    balance = balance + new_coins
    balance = balance + new_banknotes
    balance = balance + new_electronical
end

charge_balance = function(price)
    balance = balance - price * 0.001666666667
    if balance<0 then balance = 0 end
end
