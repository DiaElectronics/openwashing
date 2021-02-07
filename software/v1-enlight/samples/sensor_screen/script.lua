-- Wash Firmware

setup = function()
    turn_light(0, animation.freeze)
    -- change this constant on every post
    post_position = 1

    -- money variables
    balance = 0.0
    balance_for_enter = 0
   
    balance_seconds = 0
    cash_balance = 0.0
    electronical_balance = 0.0

    diagram_balance = 200

    -- time variables
    time_minutes = 25
    time_hours = 13

    -- temperature variables
    temp_degrees = 23
    temp_fraction = 6
    temp_is_negative = false

    -- delay variables
    thanks_mode_seconds = 20
    free_pause_seconds = 120
    wait_card_mode_seconds = 40
    show_pistol_seconds = 5
    
    frame_delay = 100

    frames_per_second = 1000 / frame_delay
    balance_reduce_rate = 1 / (frames_per_second * 60)

    -- flags
    is_transaction_started = false
    is_waiting_receipt = false
    is_showing_pistol = false
    is_sum_ready = false

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
    
    mode_thanks = 80
    
    currentMode = mode_start

    sub_mode = 0
    old_sub_mode = 0

    version = "3.0.0"

    if need_to_open_lid then printMessage("OPEN LID AGAIN!!!") end

    init_constants();

    forget_pressed_key();

    printMessage("dia proprietary QUARTZ wash firmware v." .. version)

    return 0
end

update_post = function() 
    post_position = registry:GetPostID();
end

forget_pressed_key = function()
    key = get_key()
end

loop = function()
    update_time()
    update_temp()
    update_post()
    currentMode = run_mode(currentMode)
    smart_delay(frame_delay)
    return 0
end

update_time = function()
    time_minutes = hardware:GetMinutes()
    time_hours = hardware:GetHours()
end

update_temp = function()
    temp_degrees = weather:GetTempDegrees()
    temp_fraction = weather:GetTempFraction()
    temp_is_negative = weather:IsNegative()
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
    
    if is_working_mode (new_mode) then return program_mode(new_mode) end
    if new_mode == mode_pause then return pause_mode() end
    
    if new_mode == mode_thanks then return thanks_mode() end
end

-- start_mode should show screen with all programs, their prices and text like "pay with cash/pay with bank card"
start_mode = function()
    show_start()
    run_stop()

    if need_to_open_lid() then
        open_lid()
    end

    pressed_key = get_key()
    if pressed_key == 1 then
        animate_start_cash_button()
        return mode_cash
    end

    update_balance()
    
    if balance > 0.9 then 
        return mode_cash
    end

    if pressed_key == 2 then
       animate_start_card_button()
       return mode_enter_price
    end

    return mode_start
end

-- enter_price_mode is the screen where user can enter an interesting amount of money to be charged from his card
enter_price_mode = function()
    
    show_enter_price()
    run_stop()

    pressed_key = get_key()
    if balance_for_enter > 900 or 
        (balance_for_enter < 50 and balance_for_enter > 0) then
            is_sum_ready = false end
    if pressed_key == 1 and is_sum_ready then
        animate_enter_price_begin_wash_button()
        balance_int = 0
        balance_for_enter = 0
        enter_price:Set("balance.value", balance_int)
        return mode_wait_for_card
    end
    if pressed_key == 2 then
        animate_enter_price_cancel_button()
        balance_int = 0
        balance = 0
        balance_for_enter = 0
        enter_price:Set("balance.value", balance_int)
        is_sum_ready = false
        return mode_start
    end
    if pressed_key >=11 and pressed_key <=21 or pressed_key == 3 then
        animate_enter_price_all_number_buttons(pressed_key)
        change_enter_price_selected_button_value(pressed_key)
        balance_int = math.floor(balance_for_enter)
        enter_price:Set("balance.value", balance_int)
        is_sum_ready = true
        return mode_enter_price
    end    
    
    return mode_enter_price
end

-- wait_for_card_mode is the screen saying you need to put your card to pay
wait_for_card_mode = function()
    show_wait_for_card()
    run_stop()

    if is_transaction_started == false then
        waiting_loops = wait_card_mode_seconds * frames_per_second;

        request_transaction(balance_for_enter)
        
        is_transaction_started = true
    end

    pressed_key = get_key()
    if pressed_key > 0 and pressed_key < 7 then
        animate_wait_for_card_cancel_button()
        waiting_loops = 0
    end

    status = get_transaction_status()
    update_balance()

    if balance > 0.99 then
        if status ~= 0 then 
            abort_transaction()
        end
        is_transaction_started = false
        return mode_work
    end

    if waiting_loops <= 0 then
    is_transaction_started = false
	if status ~= 0 then
	    abort_transaction()
	end
        return mode_choose_method
    end

    if (status == 0) and (is_transaction_started == true) then
        is_transaction_started = false
        abort_transaction()
        return mode_choose_method        
	end

    smart_delay(frame_delay)
    waiting_loops = waiting_loops - 1
   
    return mode_wait_for_card
end

-- cash_mode this shows your balance and asks to put some money in the coin acceptor
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

    return mode_cash
end

program_mode = function(working_mode)
  old_sub_mode = sub_mode
  sub_mode = working_mode - mode_work
  
  if sub_mode == 0 then
    run_pause()
  else
    run_program(sub_mode)
  end

  show_working(sub_mode, balance)
  
  if sub_mode == 0 then
    balance_seconds = free_pause_seconds
  end
  
  charge_balance(price_p[sub_mode])
  if balance <= 0.01 then 
    return mode_thanks 
  end
  update_balance()
  suggested_mode = get_mode_by_pressed_key()
  if is_showing_pistol == false then
    if suggested_mode >=0 then return suggested_mode end
  end
  
  return working_mode
end

pause_mode = function()
    old_sub_mode = 6
    sub_mode = 6
    show_pause(balance, balance_seconds)
    run_pause()

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
        run_pause()
        waiting_loops = thanks_mode_seconds * frames_per_second;
        is_waiting_receipt = true
    end
 
    if waiting_loops > 0 then
        show_thanks()
        pressed_key = get_key()
        if pressed_key > 0 and pressed_key < 7 then
            waiting_loops = 0
        end
        waiting_loops = waiting_loops - 1
    else
        send_receipt(post_position, cash_balance, electronical_balance)
        cash_balance = 0
        electronical_balance = 0
        is_waiting_receipt = false
        is_showing_pistol = false
        sub_mode = 0
        old_sub_mode = 0
        increment_cars()
        return mode_start	
    end

    return mode_thanks
end

show_temp = function(obj) 
    obj:Set("temp_fraction.value", temp_fraction)
    obj:Set("temp_degrees.value", temp_degrees)

    if temp_is_negative == true then 
        obj:Set("temp_sign.visible", "true") 
    else
        obj:Set("temp_sign.visible", "false") 
    end
end

show_start = function()
    start:Set("time_min.value", time_minutes)
    start:Set("time_hours.value", time_hours)

    show_temp(start)

    price1_int = math.ceil(price_p[1])
    start:Set("price_p1.value", price1_int)
    price2_int = math.ceil(price_p[2])
    start:Set("price_p2.value", price2_int)
    price3_int = math.ceil(price_p[3])
    start:Set("price_p3.value", price3_int)
    price4_int = math.ceil(price_p[4])
    start:Set("price_p4.value", price4_int)
    price5_int = math.ceil(price_p[5])
    start:Set("price_p5.value", price5_int)
    price6_int = math.ceil(price_p[6])
    start:Set("price_p6.value", price6_int)

    start:Set("post_numbers.index", post_position-1)

    start:Display()
end

show_cash = function()
    balance_to_show = balance

    if balance == 0 then
        balance_to_show = 1
    end

    if balance > diagram_balance then
        balance_to_show = diagram_balance
    end 

    index = math.ceil((balance_to_show / diagram_balance)*15 - 1)
    cash:Set("diagrams.index", index)

    cash:Set("time_min.value", time_minutes)
    cash:Set("time_hours.value", time_hours)

    show_temp(cash)

    cash:Set("post_numbers.index", post_position-1)
    balance_int = math.ceil(balance)
    cash:Set("balance.value", balance_int)

    cash:Display()
end

show_enter_price = function()
    enter_price:Set("time_min.value", time_minutes)
    enter_price:Set("time_hours.value", time_hours)

    show_temp(enter_price)

    enter_price:Set("post_numbers.index", post_position-1)
    enter_price:Display()
end

show_wait_for_card = function()
    card:Set("time_min.value", time_minutes)
    card:Set("time_hours.value", time_hours)

    show_temp(card)

    card:Set("post_numbers.index", post_position-1)
    balance_int = math.ceil(balance)
    card:Set("balance.value", balance_int)

    card:Display()
end

show_working = function(sub_mode, balance_rur)
    balance_to_show = balance_rur

    if balance_rur == 0 then
        balance_to_show = 1
    end

    if balance_rur > diagram_balance then
        balance_to_show = diagram_balance
    end 

    index = math.ceil((balance_to_show / diagram_balance)*15 - 1)
    working:Set("diagrams.index", index)

    balance_int = math.floor(balance_rur)
    working:Set("balance.value", balance_int)
    
    working:Set("time_min.value", time_minutes)
    working:Set("time_hours.value", time_hours)

    show_temp(working)

    working:Set("post_numbers.index", post_position-1)

    switch_submodes(sub_mode)
    working:Display()
end

show_pause = function(balance_rur, balance_sec)
    balance_int = math.ceil(balance_rur)
    sec_int = math.ceil(balance_sec)
    working:Set("balance.value", balance_int)
    switch_submodes(6)
    working:Display()
end

switch_submodes = function(sub_mode) 
    -- new mode swithed, so we need to show pistol
    if sub_mode ~= old_sub_mode and sub_mode ~= 6 then
        working:Set("button_p1_on.visible", "false")
        working:Set("button_p2_on.visible", "false")
        working:Set("button_p3_on.visible", "false")
        working:Set("button_p4_on.visible", "false")
        working:Set("button_p5_on.visible", "false")
        working:Set("button_p6_on.visible", "false")

        if sub_mode == 1 then animate_p1_button() else working:Set("button_p1_on.visible", "false") end
        if sub_mode == 2 then animate_p2_button() else working:Set("button_p2_on.visible", "false") end
        if sub_mode == 3 then animate_p3_button() else working:Set("button_p3_on.visible", "false") end
        if sub_mode == 4 then animate_p4_button() else working:Set("button_p4_on.visible", "false") end
        if sub_mode == 5 then animate_p5_button() else working:Set("button_p5_on.visible", "false") end
        if sub_mode == 6 then animate_p6_button() else working:Set("button_p6_on.visible", "false") end

        is_showing_pistol = true

        pistol_waiting_loops = show_pistol_seconds * frames_per_second;

        working:Set("button_p1.visible", "false")
        working:Set("button_p2.visible", "false")
        working:Set("button_p3.visible", "false")
        working:Set("button_p4.visible", "false")
        working:Set("button_p5.visible", "false")
        working:Set("button_p6.visible", "false")

        if sub_mode == 1 then
            working:Set("red_pistol.visible", "true")
        else
            working:Set("blue_pistol.visible", "true")
        end
    end

    if is_showing_pistol == true then
        if pistol_waiting_loops > 0 then
            pistol_waiting_loops = pistol_waiting_loops - 1
        else
            is_showing_pistol = false

            working:Set("button_p1.visible", "true")
            working:Set("button_p2.visible", "true")
            working:Set("button_p3.visible", "true")
            working:Set("button_p4.visible", "true")
            working:Set("button_p5.visible", "true")
            working:Set("button_p6.visible", "true")

            if sub_mode == 1 then
                working:Set("red_pistol.visible", "false")
            else
                working:Set("blue_pistol.visible", "false")
            end

            if sub_mode == 1 then working:Set("button_p1_on.visible", "true") else working:Set("button_p1_on.visible", "false") end
            if sub_mode == 2 then working:Set("button_p2_on.visible", "true") else working:Set("button_p2_on.visible", "false") end
            if sub_mode == 3 then working:Set("button_p3_on.visible", "true") else working:Set("button_p3_on.visible", "false") end
            if sub_mode == 4 then working:Set("button_p4_on.visible", "true") else working:Set("button_p4_on.visible", "false") end
            if sub_mode == 5 then working:Set("button_p5_on.visible", "true") else working:Set("button_p5_on.visible", "false") end
            if sub_mode == 6 then working:Set("button_p6_on.visible", "true") else working:Set("button_p6_on.visible", "false") end
        end     
    else 
        if sub_mode == 1 then working:Set("button_p1_on.visible", "true") else working:Set("button_p1_on.visible", "false") end
        if sub_mode == 2 then working:Set("button_p2_on.visible", "true") else working:Set("button_p2_on.visible", "false") end
        if sub_mode == 3 then working:Set("button_p3_on.visible", "true") else working:Set("button_p3_on.visible", "false") end
        if sub_mode == 4 then working:Set("button_p4_on.visible", "true") else working:Set("button_p4_on.visible", "false") end
        if sub_mode == 5 then working:Set("button_p5_on.visible", "true") else working:Set("button_p5_on.visible", "false") end
        if sub_mode == 6 then working:Set("button_p6_on.visible", "true") else working:Set("button_p6_on.visible", "false") end
    end
end

show_thanks =  function()
    thanks:Set("time_min.value", time_minutes)
    thanks:Set("time_hours.value", time_hours)
    
    show_temp(thanks)

    thanks:Set("post_numbers.index", post_position-1)

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

animate_enter_price_numbers_buttons = function(pressedkey) -- non working method
    if pressedkey == 3 then 
        enter_price:Set("button_button_1_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_button_1_on.visible", "false")
        enter_price:Display()
    end
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

animate_enter_price_all_number_buttons = function(pressed_key_var)
    if pressed_key_var == 11 then
        enter_price:Set("button_1_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_1_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 12 then
        enter_price:Set("button_2_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_2_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 13 then
        enter_price:Set("button_3_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_3_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 14 then
        enter_price:Set("button_4_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_4_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 15 then
        enter_price:Set("button_5_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_5_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 16 then
        enter_price:Set("button_6_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_6_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 17 then
        enter_price:Set("button_7_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_7_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 18 then
        enter_price:Set("button_8_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_8_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 19 then
        enter_price:Set("button_9_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_9_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 20 then
        enter_price:Set("button_0_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_0_on.visible", "false")

        enter_price:Display()
    end
    if pressed_key_var == 21 then
        enter_price:Set("button_delete_on.visible", "true")
        enter_price:Display()
        smart_delay(500)
        enter_price:Set("button_delete_on.visible", "false")

        enter_price:Display()
    end


end

animate_p1_button = function()
    working:Set("button_p1_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p1_on.visible", "false")
    working:Display()
end

animate_p2_button = function()
    working:Set("button_p2_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p2_on.visible", "false")
    working:Display()
end

animate_p3_button = function()
    working:Set("button_p3_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p3_on.visible", "false")
    working:Display()
end

animate_p4_button = function()
    working:Set("button_p4_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p4_on.visible", "false")
    working:Display()
end

animate_p5_button = function()
    working:Set("button_p5_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p5_on.visible", "false")
    working:Display()
end

animate_p6_button = function()
    working:Set("button_p6_on.visible", "true")
    working:Display()
    smart_delay(700)
    working:Set("button_p6_on.visible", "false")
    working:Display()
end

change_enter_price_selected_button_value = function(pressed_key_var)
    if pressed_key_var == 11 then
        balance_for_enter = balance_for_enter*10 + 1
    end
    if pressed_key_var == 12 then
        balance_for_enter = balance_for_enter*10 + 2
    end
    if pressed_key_var == 13 then
        balance_for_enter = balance_for_enter*10 + 3
    end
    if pressed_key_var == 14 then
        balance_for_enter = balance_for_enter*10 + 4
    end
    if pressed_key_var == 15 then
        balance_for_enter = balance_for_enter*10 + 5
    end
    if pressed_key_var == 16 then
        balance_for_enter = balance_for_enter*10 + 6
    end
    if pressed_key_var == 17 then
        balance_for_enter = balance_for_enter*10 + 7
    end
    if pressed_key_var == 18 then
        balance_for_enter = balance_for_enter*10 + 8
    end
    if pressed_key_var == 19 then
        balance_for_enter = balance_for_enter*10 + 9
    end
    if pressed_key_var == 20 then
        balance_for_enter = balance_for_enter*10 + 0
    end
    if pressed_key_var == 21 then 
        balance_for_enter = balance_for_enter/10 --delete number
    end
    return mode_enter_price
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

send_receipt = function(post_pos, cash, electronical)
    hardware:SendReceipt(post_pos, cash, electronical)
end

increment_cars = function()
    hardware:IncrementCars()
end

run_pause = function()
    run_program(program.p6relay)
end

run_stop = function()
    if balance>0.9 then run_pause() end
    if balance<=0.9 then run_program(program.stop) end
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

    cash_balance = cash_balance + new_coins
    cash_balance = cash_balance + new_banknotes
    electronical_balance = electronical_balance + new_electronical
    balance = balance + new_coins
    balance = balance + new_banknotes
    balance = balance + new_electronical
    balance = balance + new_service
end

charge_balance = function(price)
    balance = balance - price * balance_reduce_rate
    if balance<0 then balance = 0 end
end

is_working_mode = function(mode_to_check)
  if mode_to_check >= mode_work and mode_to_check<mode_work+10 then return true end
  return false
end

need_to_open_lid = function()
    val = hardware:GetOpenLid()
    if val>0 then return true end
    return false
end

open_lid = function()
    printMessage("LID OPENED !!!")
    open_lid_program = program.p7openlid
    printMessage("LID OPENED ...")
    run_program(open_lid_program)
    printMessage("LID OPENED :(")
    smart_delay(20000)
    printMessage("LID CLOSED :)")
end
