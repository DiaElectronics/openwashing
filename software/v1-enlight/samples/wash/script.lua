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

    price_p1 = 0
    price_p2 = 0
    price_p3 = 0
    price_p4 = 0
    price_p5 = 0
    price_p6 = 0

    init_prices()
    
    mode_welcome = 0
    mode_choose_method = 10
    mode_select_price = 20
    mode_wait_for_card = 30
    mode_ask_for_money = 40
    mode_start = 50
    mode_p1 = 60
    mode_p2 = 70
    mode_p3 = 80
    mode_p4 = 90
    mode_p5 = 100
    mode_p6 = 110
    mode_thanks = 120
    
    currentMode = mode_welcome
    version = "1.0.0"

    printMessage("dia generic wash firmware v." .. version)
    return 0
end

init_prices = function()
    

    price_p1 = get_price("price1")
    if price_p1 == 0 then price_p1 = 18 end

    price_p2 = get_price("price2")
    if price_p2 == 0 then price_p2 = 18 end

    price_p3 = get_price("price3")
    if price_p3 == 0 then price_p3 = 18 end

    price_p4 = get_price("price4")
    if price_p4 == 0 then price_p4 = 18 end

    price_p5 = get_price("price5")
    if price_p5 == 0 then price_p5 = 18 end
    
    price_p6 = get_price("price6")
    if price_p6 == 0 then price_p6 = 18 end
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
    if new_mode == mode_p1 then return p1_mode() end
    if new_mode == mode_p2 then return p2_mode() end
    if new_mode == mode_p3 then return p3_mode() end
    if new_mode == mode_p4 then return p4_mode() end
    if new_mode == mode_p5 then return p5_mode() end
    if new_mode == mode_p6 then return p6_mode() end
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
    run_p6()
    turn_light(0, animation.intense)
    balance_seconds = free_pause_seconds    
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    -- user has no reason to start with pause here
    if suggested_mode >= 0 and suggested_mode~=mode_pause then return suggested_mode end
    return mode_start
end

p1_mode = function()
    show_p1(balance)
    run_p1()
    turn_light(1, animation.one_button)
    charge_balance(price_p1)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p1
end

p2_mode = function()
    show_p2(balance)
    run_p2()
    turn_light(2, animation.one_button)
    charge_balance(price_p2)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p2
end

p3_mode = function()
    show_p3(balance)
    run_p3()
    turn_light(3, animation.one_button)
    charge_balance(price_p3)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p3
end

p4_mode = function()
    show_p4(balance)
    run_p4()
    turn_light(4, animation.one_button)
    charge_balance(price_p4)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p4
end

p5_mode = function()
    show_p5(balance)
    run_p5()
    turn_light(5, animation.one_button)
    charge_balance(price_p5)
    if balance <= 0.01 then return mode_thanks end
    update_balance()
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p5
end


p6_mode = function()
    show_p6(balance, balance_seconds)
    run_p6()
    turn_light(6, animation.one_button)
    update_balance()
    if balance_seconds > 0 then
        balance_seconds = balance_seconds - 0.1
    else
        balance_seconds = 0
        charge_balance(price_p6)         
    end
    
    if balance <= 0.01 then return mode_thanks end
    
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_p6
end

thanks_mode = function()
    balance = 0
    show_thanks(thanks_mode_seconds)
    turn_light(1, animation.one_button)
    run_program(program.p6relay)
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
    ask_for_money:Display()
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

show_p1 = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    p1screen:Set("balance.value", balance_int)
    p1screen:Display()
end

show_p2 = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    p2screen:Set("balance.value", balance_int)
    p2screen:Display()
end

show_p3 = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    p3screen:Set("balance.value", balance_int)
    p3screen:Display()
end

show_p4 = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    p4screen:Set("balance.value", balance_int)
    p4screen:Display()
end

show_p5 = function(balance_rur)
    balance_int = math.ceil(balance_rur)
    p5screen:Set("balance.value", balance_int)
    p5screen:Display()
end

show_p6 = function(balance_rur, balance_sec)
    balance_int = math.ceil(balance_rur)
    sec_int = math.ceil(balance_sec)
    p6screen:Set("pause_balance.value", sec_int)
    p6screen:Set("balance.value", balance_int)
    p6screen:Display()
end



show_thanks =  function(seconds_float)
    seconds_int = math.ceil(seconds_float)
    thanks:Set("delay_seconds.value", seconds_int)
    thanks:Display()
end

get_mode_by_pressed_key = function()
    pressed_key = get_key()
    if pressed_key == 1 then return mode_p1 end
    if pressed_key == 2 then return mode_p2 end
    if pressed_key == 3 then return mode_p3 end
    if pressed_key == 4 then return mode_p4 end
    if pressed_key == 5 then return mode_p5 end
    if pressed_key == 6 then return mode_p6 end
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

run_p1 = function()
    run_program(program.p1relay)
end

run_p2 = function()
    run_program(program.p2relay)
end

run_p3 = function()
    run_program(program.p3relay)
end

run_p4 = function()
    run_program(program.p4relay)
end

run_p5 = function()
    run_program(program.p5relay)
end

run_p6 = function()
    run_program(program.p6relay)
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
