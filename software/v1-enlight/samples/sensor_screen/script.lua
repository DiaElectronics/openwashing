-- Wash Firmware

setup = function()
    -- global variables
    balance = 0.0

    electron_balance = 0
    balance_seconds = 0
    kasse_balance = 0.0
    post_position = 1      

    thanks_mode_seconds = 20
    free_pause_seconds = 120
    wait_card_mode_seconds = 40
    
    is_transaction_started = false
    is_waiting_receipt = false

    price_p = {}
    
    price_p[0] = 0
    price_p[1] = 0
    price_p[2] = 0
    price_p[3] = 0
    price_p[4] = 0
    price_p[5] = 0
    price_p[6] = 0

    init_prices()
    
    mode_start = 0
    mode_cash = 10
    mode_enter_price = 20
    mode_wait_for_card = 30
    
    -- all these modes MUST follow each other
    mode_work = 40
    mode_pause = 50
    -- end of modes which MUST follow each other

    mode_use_red = 60
    mode_use_blue = 70
    
    mode_thanks = 80
    mode_not_working = 90
    
    currentMode = mode_start

    version = "3.0.0"

    printMessage("dia generic wash firmware v." .. version)

    index = 0

    return 0
end

loop = function()
    currentMode = run_mode(currentMode)
    smart_delay(100)
    return 0
end

init_prices = function()
    price_p[1] = get_price("price1")
    if price_p[1] == 0 then price_p[1] = 18 end

    price_p[2] = get_price("price2")
    if price_p[2] == 0 then price_p[2] = 18 end

    price_p[3] = get_price("price3")
    if price_p[3] == 0 then price_p[3] = 18 end

    price_p[4] = get_price("price4")
    if price_p[4] == 0 then price_p[4] = 18 end

    price_p[5] = get_price("price5")
    if price_p[5] == 0 then price_p[5] = 18 end
    
    price_p[6] = get_price("price6")
    if price_p[6] == 0 then price_p[6] = 18 end
end

run_mode = function(new_mode)   
    if new_mode == mode_start then return start_mode() end
    if new_mode == mode_cash then return cash_mode() end
    if new_mode == mode_enter_price then return enter_price_mode() end
    if new_mode == mode_wait_for_card then return wait_for_card_mode() end
    
    -- TO DO: check working mode
    if is_working_mode (new_mode) then return program_mode(new_mode) end
    if new_mode == mode_pause then return pause_mode() end

    if new_mode == mode_use_red then return use_red_mode() end
    if new_mode == mode_use_blue then return use_blue_mode() end
    
    if new_mode == mode_thanks then return thanks_mode() end
    if new_mode == mode_not_working then return not_working_mode() end
end

start_mode = function()
    show_start()
    run_stop()

    pressed_key = get_key()
    if pressed_key == 1 then
        animate_start_cash_button()
        return mode_cash
    end
    if pressed_key == 2 then
        animate_start_card_button()
        return mode_enter_price
    end

    return mode_start
end

enter_price_mode = function()
    show_enter_price()
    run_stop()

    -- TO DO: read the screen keyboard here
    
    pressed_key = get_key()
    if pressed_key == 1 then
        animate_enter_price_begin_wash_button()
        return mode_wait_for_card
    end
    if pressed_key == 2 then
        animate_enter_price_cancel_button()
        return mode_start
    end
    
    return mode_enter_price
end

wait_for_card_mode = function()
    show_wait_for_card()
    run_stop()

    if is_transaction_started == false then
        waiting_loops = wait_card_mode_seconds * 10;

        request_transaction(electron_balance)
        electron_balance = min_electron_balance
        is_transaction_started = true
    end

    pressed_key = get_key()
    if pressed_key > 0 and pressed_key < 7 then
        animate_wait_for_card_cancel_button()
        waiting_loops = 0
    end

    update_balance()
    if balance > 0.99 then
        status = get_transaction_status()
        if status ~= 0 then 
            abort_transaction()
        end
        is_transaction_started = false
        return mode_work
    end

    if waiting_loops <= 0 then
        is_transaction_started = false
	    status = get_transaction_status()
	    if status ~= 0 then
	        abort_transaction()
	    end
        return mode_start
    end

    smart_delay(100)
    waiting_loops = waiting_loops - 1
   
    return mode_wait_for_card
end

cash_mode = function()
    show_cash()
    run_stop()
    
    update_balance()

    pressed_key = get_key()
    if pressed_key == 1 then
        if balance > 0.99 then
            animate_cash_begin_wash_button()
            return mode_work
        end
    end
    if pressed_key == 2 then
        animate_cash_cancel_button()
        return mode_start
    end
    
    -- TO DO: show amount of money on the screen
    return mode_cash
end

program_mode = function(working_mode)
  sub_mode = working_mode - mode_work
  run_program(sub_mode)
  show_working(sub_mode, balance)
  
  if sub_mode == 0 then
    balance_seconds = free_pause_seconds
    turn_light(0, animation.intense)
  else
    turn_light(sub_mode, animation.one_button)
  end
  
  charge_balance(price_p[sub_mode])
  if balance <= 0.01 then 
    return mode_thanks 
  end
  update_balance()
  suggested_mode = get_mode_by_pressed_key()
  if suggested_mode >=0 then return suggested_mode end
  return working_mode
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
        charge_balance(price_p[6])
    end
    
    if balance <= 0.01 then return mode_thanks end
    
    suggested_mode = get_mode_by_pressed_key()
    if suggested_mode >=0 then return suggested_mode end
    return mode_pause
end

thanks_mode = function()
    if is_waiting_receipt == false then
        balance = 0
        show_thanks()
        turn_light(1, animation.one_button)
        run_pause()
        waiting_loops = thanks_mode_seconds * 10;
        is_waiting_receipt = true
    end
 
    if waiting_loops > 0 then
        show_thanks()
        pressed_key = get_key()
        if pressed_key > 0 and pressed_key < 7 then
            waiting_loops = 0
        end
        update_balance()
        if balance > 0.99 then
            send_receipt(post_position, 0, kasse_balance)
            kasse_balance = 0
            is_waiting_receipt = false
            increment_cars() 
            return mode_work 
        end
        waiting_loops = waiting_loops - 1
    else
        send_receipt(post_position, 0, kasse_balance)
        kasse_balance = 0
        is_waiting_receipt = false
        increment_cars()
        return mode_start	
    end

    return mode_thanks
end

show_start = function()
    start:Set("time_min.value", 25)
    start:Set("time_hours.value", 13)
    start:Set("temp_fraction.value", 6)
    start:Set("temp_degrees.value", 34)
    start:Set("price_p1.value", 18)
    start:Set("price_p2.value", 15)
    start:Set("price_p3.value", 15)
    start:Set("price_p4.value", 18)
    start:Set("price_p5.value", 15)
    start:Set("price_p6.value", 10)
    start:Set("post_numbers.index", index)
    index = index + 1
    if index == 12 then
        index = 0
    end
    start:Display()
end

show_cash = function()
    cash:Display()
end

show_enter_price = function()
    enter_price:Display()
end

show_wait_for_card = function()
    card:Display()
end

show_working = function(sub_mode, balance_rur)
    balance_int = math.ceil(balance_rur)
    working:Set("pause_digits.visible", "false")
    working:Set("balance.value", balance_int)
    
    switch_submodes(sub_mode)
    working:Display()
end

show_pause = function(balance_rur, balance_sec)
    balance_int = math.ceil(balance_rur)
    sec_int = math.ceil(balance_sec)
    working:Set("pause_digits.visible", "true")
    working:Set("pause_digits.value", sec_int)
    working:Set("balance.value", balance_int)
    switch_submodes(6)
    working:Display()
end

switch_submodes = function(sub_mode) 
    if sub_mode == 1 then working:Set("p1.visible", "true") else working:Set("p1.visible", "false") end
    if sub_mode == 2 then working:Set("p2.visible", "true") else working:Set("p2.visible", "false") end
    if sub_mode == 3 then working:Set("p3.visible", "true") else working:Set("p3.visible", "false") end
    if sub_mode == 4 then working:Set("p4.visible", "true") else working:Set("p4.visible", "false") end
    if sub_mode == 5 then working:Set("p5.visible", "true") else working:Set("p5.visible", "false") end
    if sub_mode == 6 then working:Set("p6.visible", "true") else working:Set("p6.visible", "false") end
    
end

show_thanks =  function(seconds_float)
    thanks:Display()
end

animate_start_cash_button = function()
    start:Set("button_cash_on.visible", "true")
    start:Display()
    smart_delay(500)
    start:Set("button_cash_on.visible", "false")
    start:Display()
end

animate_start_card_button = function()
    start:Set("button_card_on.visible", "true")
    start:Display()
    smart_delay(500)
    start:Set("button_card_on.visible", "false")
    start:Display()
end

animate_cash_begin_wash_button = function()
    cash:Set("button_begin_washing_on.visible", "true")
    cash:Display()
    smart_delay(500)
    cash:Set("button_begin_washing_on.visible", "false")
    cash:Display()
end

animate_cash_cancel_button = function()
    cash:Set("button_cancel_on.visible", "true")
    cash:Display()
    smart_delay(500)
    cash:Set("button_cancel_on.visible", "false")
    cash:Display()
end

animate_enter_price_begin_wash_button = function()
    enter_price:Set("button_begin_washing_on.visible", "true")
    enter_price:Display()
    smart_delay(500)
    enter_price:Set("button_begin_washing_on.visible", "false")
    enter_price:Display()
end

animate_enter_price_cancel_button = function()
    enter_price:Set("button_cancel_on.visible", "true")
    enter_price:Display()
    smart_delay(500)
    enter_price:Set("button_cancel_on.visible", "false")
    enter_price:Display()
end

animate_wait_for_card_cancel_button = function()
    card:Set("button_cancel_on.visible", "true")
    card:Display()
    smart_delay(500)
    card:Set("button_cancel_on.visible", "false")
    card:Display()
end

get_mode_by_pressed_key = function()
    pressed_key = get_key()
    if pressed_key >= 1 and pressed_key<=5 then return mode_work + pressed_key end
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

increment_cars = function()
    hardware:IncrementCars()
end

run_pause = function()
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
    new_service = hardware:GetService()

    kasse_balance = kasse_balance + new_coins
    kasse_balance = kasse_balance + new_banknotes
    kasse_balance = kasse_balance + new_electronical
    kasse_balance = kasse_balance + new_service
    balance = balance + new_coins
    balance = balance + new_banknotes
    balance = balance + new_electronical
    balance = balance + new_service
end

charge_balance = function(price)
    balance = balance - price * 0.001666666667
    if balance<0 then balance = 0 end
end

is_working_mode = function(mode_to_check)
  if mode_to_check >= mode_work and mode_to_check<mode_work+10 then return true end
  return false
end
