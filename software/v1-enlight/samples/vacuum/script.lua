-- Wash Firmware

-- setup is running at the start just once
setup = function()
    -- global variables
    balance = 0.0

    min_electron_balance = 10
    max_electron_balance = 900
    electron_amount_step = 10
    electron_balance = min_electron_balance
    free_pause_seconds = 0
    
    balance_seconds = 0
    kasse_balance = 0.0
    post_position = 1      

    -- constants
    welcome_mode_seconds = 3
    thanks_mode_seconds = 0
    wait_card_mode_seconds = 40
    
    hascardreader = false
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
    
    mode_welcome = 0
    mode_choose_method = 10
    mode_select_price = 20
    mode_wait_for_card = 30
    mode_ask_for_money = 40
    
    -- all these modes MUST follow each other
    mode_work = 50
    mode_pause = 60
    -- end of modes which MUST follow each other
    
    mode_thanks = 120
    
    currentMode = mode_welcome

    version = "2.1.0"

    printMessage("dia generic wash firmware v." .. version)
    return 0
end

-- loop is being executed
loop = function()
    currentMode = run_mode(currentMode)
    smart_delay(100)
    return 0
end

init_prices = function()
    price_p[1] = get_price("price1")
    if price_p[1] == 0 then price_p[1] = 12 end

    price_p[2] = get_price("price2")
    if price_p[2] == 0 then price_p[2] = 12 end

    price_p[3] = get_price("price3")
    if price_p[3] == 0 then price_p[3] = 12 end

    price_p[4] = get_price("price4")
    if price_p[4] == 0 then price_p[4] = 12 end

    price_p[5] = get_price("price5")
    if price_p[5] == 0 then price_p[5] = 12 end
    
    price_p[6] = get_price("price6")
    if price_p[6] == 0 then price_p[6] = 12 end
end



run_mode = function(new_mode)   
    if new_mode == mode_welcome then return welcome_mode() end
    if new_mode == mode_choose_method then return choose_method_mode() end
    if new_mode == mode_select_price then return select_price_mode() end
    if new_mode == mode_wait_for_card then return wait_for_card_mode() end
    if new_mode == mode_ask_for_money then return ask_for_money_mode() end
    
    if is_working_mode (new_mode) then return program_mode(new_mode) end
    if new_mode == mode_pause then return pause_mode() end
    
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
    if pressed_key == 4 or pressed_key == 5 or pressed_key == 6 then
        return mode_select_price
    end
    if pressed_key == 1 or pressed_key == 2 or pressed_key == 3 then
        return mode_ask_for_money
    end
    
    -- if someone put some money let's switch the mode.
    -- this should be rebuilt
    update_balance()
    if balance > 0.99 then
        return mode_work
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
    --return to choose method
    if pressed_key == 3 then
        return mode_choose_method
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

    if is_transaction_started == false then
        waiting_loops = wait_card_mode_seconds * 10;

        request_transaction(electron_balance)
        electron_balance = min_electron_balance
        is_transaction_started = true
    end

    pressed_key = get_key()
    if pressed_key > 0 and pressed_key < 7 then
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
        return mode_choose_method
    end

    smart_delay(100)
    waiting_loops = waiting_loops - 1
   
    return mode_wait_for_card
end

ask_for_money_mode = function()
    show_ask_for_money()
    run_stop()
    turn_light(0, animation.idle)
    
    if hascardreader == true then
        pressed_key = get_key()
        if pressed_key > 0 and pressed_key < 7 then
            return mode_choose_method
        end
    end

    update_balance()
    if balance > 0.99 then
        return mode_work
    end
    return mode_ask_for_money
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
  suggested_mode = get_mode_by_pressed_key(working_mode)
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
    
    suggested_mode = get_mode_by_pressed_key(mode_pause)
    if suggested_mode >=0 then return suggested_mode end
    return mode_pause
end

thanks_mode = function()
    if is_waiting_receipt == false then
        balance = 0
        show_thanks(thanks_mode_seconds)
        turn_light(1, animation.one_button)
        run_pause()
        waiting_loops = thanks_mode_seconds * 10;
        is_waiting_receipt = true
    end
 
    if waiting_loops > 0 then
        show_thanks(waiting_loops/10)
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
	if hascardreader == true then
        	return mode_choose_method
    	end
        return mode_ask_for_money
    end

    return mode_thanks
end

show_welcome = function()
    welcome:Display()
end

show_ask_for_money = function()
    if hascardreader == true then
        ask_for_money:Set("return_background.visible", "true")
    end
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

show_working = function(sub_mode, balance_rur)
    balance_int = math.ceil(balance_rur)
    working:Set("balance.value", balance_int)
    if sub_mode>0 then switch_submodes(1) else switch_submodes(0) end    
    working:Display()
end

show_pause = function(balance_rur, balance_sec)
    balance_int = math.ceil(balance_rur)
    sec_int = math.ceil(balance_sec)
    working:Set("balance.value", balance_int)
    switch_submodes(2)
    working:Display()
end

show_thanks =  function(seconds_float)
    seconds_int = math.ceil(seconds_float)
    thanks:Set("delay_seconds.value", seconds_int)
    thanks:Display()
end

get_mode_by_pressed_key = function(current_mode)
    pressed_key = get_key()
    
    if pressed_key < 1 then return -1 end
    if is_working_mode(current_mode) and current_mode~=mode_work then return mode_pause end
    return mode_work + 1
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

switch_submodes = function(val)
    if val == 1 then working:Set("p1_img.visible", "true") else working:Set("p1_img.visible", "false") end 
    if val == 2 then working:Set("p6_img.visible", "true") else working:Set("p6_img.visible", "false") end 
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
