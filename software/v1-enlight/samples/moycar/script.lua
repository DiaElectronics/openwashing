--display screens

setup = function()

    -- program to turn on when user paid money but has not selected a program
    default_paid_program = 8

    min_electron_balance = 50
    max_electron_balance = 900
    electron_amount_step = 25
    electron_balance = min_electron_balance
    price_p = {}
  
    price_p[0] = 0
    price_p[1] = 0
    price_p[2] = 0
    price_p[3] = 0
    price_p[4] = 0
    price_p[5] = 0
    price_p[6] = 0
    price_p[7] = 0
    price_p[8] = 0
  
    init_prices()
    set_defaults()

currentMode = mode_contract1
    
    -- external constants
    init_constants();
    forget_pressed_key();
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

    price_p[7] = get_price("price7")
    if price_p[7] == 0 then price_p[7] = 18 end
    
    price_p[8] = get_price("price8")
    if price_p[8] == 0 then price_p[8] = 18 end
end

run_mode = function(new_mode)   
  --all screens
  if new_mode == mode_contract1 then return contract1_mode() end   
  if new_mode == mode_contract2 then return contract2_mode() end
  if new_mode == mode_contract3 then return contract3_mode() end
  if new_mode == mode_contract4 then return contract4_mode() end
  if new_mode == mode_contract5 then return contract5_mode() end
  if new_mode == mode_choose_program then return choose_program_mode() end
  if new_mode == mode_choose_add_services then return choose_add_services_mode() end
  if new_mode == mode_choose_payment_method then return choose_payment_method_mode() end
  if new_mode == mode_payment_cash then return payment_cash_mode() end
  if new_mode == mode_payment_bank_card then return payment_bank_card_mode() end
  if new_mode == mode_finish then return contract1_mode() end
  if new_mode == mode_washing_started then return washing_started_mode() end
end

run_need_program = function(main_program, vacuum_cleaner_and_mats_program,interior_and_wheels_program,tire_program,disks_program,drying_program) 
    if main_program == "express" then 
	run_program(program.p1relay)
	debug_message("RAB1") 
    end
    if main_program == "daily" then 
	run_program(program.p2relay)
	debug_message("RAB2")
    end
    if main_program == "premium" then 
	run_program(program.p3relay)
  	debug_message("RAB3")
    end
    smart_delay(short_delay_var)
    run_stop()    
    if vacuum_cleaner_and_mats_program == "vacuum_cleaner_and_mats" then 
	run_program(program.p4relay)
	debug_message("RAB4")
	smart_delay(short_delay_var)
        run_stop()
    end
    if interior_and_wheels_program == "interior_and_wheels" then 
	run_program(program.p5relay)
	debug_message("RAB5")
	smart_delay(short_delay_var)
        run_stop()
    end
    if tire_program == "tire" then 
	run_program(program.p6relay)
	debug_message("RAB6")
	smart_delay(short_delay_var)
        run_stop()
    end
    if disks_program == "disks" then 
	run_program(program.p7relay)
	debug_message("RAB7")
	smart_delay(short_delay_var)
        run_stop()
    end
    if drying_program == "drying" then 
	run_program(program.p8relay)
	debug_message("RAB8")
	smart_delay(short_delay_var)
        run_stop()
    end

    printMessage("WARNING WRONG PROGRAM PASSED") 
    
end

contract1_mode = function()
  set_defaults()
  show_contract1()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract2 end
  if pressed_key == 2 then return mode_choose_program end
  return mode_contract1
end

contract2_mode = function()
  show_contract2()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract1 end
  if pressed_key == 2 then return mode_contract3 end
  return mode_contract2
end

contract3_mode = function()
  show_contract3()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract2 end
  if pressed_key == 2 then return mode_contract4 end
  return mode_contract3
end

contract4_mode = function()
  show_contract4()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract3 end
  if pressed_key == 2 then return mode_contract5 end
  return mode_contract4
end

contract5_mode = function()
  show_contract5()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract4 end
  if pressed_key == 2 then return mode_choose_program end
  return mode_contract5
end

choose_program_mode = function()
  show_choose_program()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_contract5 end
  if pressed_key == 2 and (main_chosen_program == "express" or main_chosen_program == "daily" or main_chosen_program == "premium") then
    return mode_choose_add_services end
  if pressed_key == 3 then express() end 
  if pressed_key == 4 then daily() end 
  if pressed_key == 5 then premium() end 
  return mode_choose_program
end 

choose_add_services_mode = function()
  show_choose_add_services()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_choose_program end
  if pressed_key == 2 then 
	summ_cost_var = 0 
	--counting main program cost
	if main_chosen_program == "express" then summ_cost_var = summ_cost_var + express_cost end
	if main_chosen_program == "daily" then summ_cost_var = summ_cost_var + daily_cost end
	if main_chosen_program == "premium" then summ_cost_var = summ_cost_var + premium_cost end 
	  
	--counting add service cost
	if vacuum_cleaner_and_mats_var == "vacuum_cleaner_and_mats" then summ_cost_var = summ_cost_var + vacuum_cleaner_and_mats_cost end
	if interior_and_wheels_var == "interior_and_wheels" then summ_cost_var = summ_cost_var + interior_and_wheels_cost end
	if tire_var == "tire" then summ_cost_var = summ_cost_var + tire_cost  end
        if disks_var == "disks" then summ_cost_var = summ_cost_var + disks_cost  end
	if drying_var == "drying" then summ_cost_var = summ_cost_var + drying_cost end
	return mode_choose_payment_method 
  end
  if pressed_key == 3 then vacuum_cleaner_and_mats() end
  if pressed_key == 4 then interior_and_wheels() end
  if pressed_key == 5 then tire()  end
  if pressed_key == 6 then disks() end
  if pressed_key == 7 then drying() end
  return mode_choose_add_services
end

choose_payment_method_mode = function()
  show_choose_payment_method()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_choose_add_services end
  if pressed_key == 2 and payment_method_var == "cash" then return mode_payment_cash end
  if pressed_key == 2 and payment_method_var == "bank_card" then return mode_payment_bank_card end
  if pressed_key == 3 then cash() end
  if pressed_key == 4 then bank_card() end
  return mode_choose_payment_method
end

wait_for_card = function()
  debug_message("{{{{{{{{{{{{{{ start wait for card")
    run_stop()

    if is_transaction_started == false then
        waiting_loops = wait_card_mode_seconds * 10;

        request_transaction(summ_cost_var)
        is_transaction_started = true
    end
    update_balance()
    if balance > 0.99 then
        status = get_transaction_status()
        if status ~= 0 then 
            abort_transaction()
        end
        is_transaction_started = false
        electronical_balance = balance
    end
    if waiting_loops <= 0 then
        is_transaction_started = false
	status = get_transaction_status()
	if status ~= 0 then
	    abort_transaction()
	end

end

    smart_delay(100)
    waiting_loops = waiting_loops - 1
   
    return mode_payment_bank_card 
end

payment_cash_mode = function()
  update_balance()
  if cash_balance >= summ_cost_var then
    payment:Set("button_to_pay_off.visible", "false")
    payment:Set("button_to_pay.visible", "true")
  end
  show_payment_cash()
  pressed_key = get_key()
  if pressed_key == 1 then return mode_choose_payment_method end
  if pressed_key == 2 and cash_balance >= summ_cost_var then 
    run_need_program(main_chosen_program, vacuum_cleaner_and_mats_var,interior_and_wheels_var,tire_var,disks_var,drying_var)  
    return mode_washing_started
  end
  return mode_payment_cash 
end

payment_bank_card_mode = function()
  update_balance()
  if cash_balance >= summ_cost_var then
    payment:Set("button_to_pay_off.visible", "false")
    payment:Set("button_to_pay.visible", "true")
  end
  show_payment_electronical()
  wait_for_card()
  pressed_key = get_key()
  if pressed_key == 1 then
    debug_message("return mode_choose_payment_method") 
    return mode_choose_payment_method
  end
  if pressed_key == 2 and electronical_balance >= summ_cost_var then
    debug_message("run_need_program")  
    run_need_program(main_chosen_program, vacuum_cleaner_and_mats_var,interior_and_wheels_var,tire_var,disks_var,drying_var)  
    return mode_washing_started
  end
  return mode_payment_bank_card 
end

washing_started_mode = function()
  show_washing_started()
  smart_delay(10000)
  return mode_finish
end

set_button_state = function(choose_program,name_buttons,is_on)
  off_visible = "true"
  on_visible = "false"
  if is_on then 
     off_visible = "false"
     on_visible = "true"
  end
  choose_program:Set("button_" .. name_buttons .. "_off.visible", off_visible)
  choose_program:Set("button_" .. name_buttons .. "_on.visible", on_visible)
end

cash = function()
  set_button_state(choose_payment_method,"cash",true)
  set_button_state(choose_payment_method,"bank_card",false)
  payment_method_var = "cash"
end

bank_card = function()
  set_button_state(choose_payment_method,"cash",false)
  set_button_state(choose_payment_method,"bank_card",true)
  payment_method_var = "bank_card"
end

express = function()
  set_button_state(choose_program,"express",true)
  set_button_state(choose_program,"daily",false)
  set_button_state(choose_program,"premium",false)
  main_chosen_program = "express"
end 

daily = function()
  set_button_state(choose_program,"express",false)
  set_button_state(choose_program,"daily",true)
  set_button_state(choose_program,"premium",false)
  main_chosen_program = "daily"
end

premium = function()
  set_button_state(choose_program,"express",false)
  set_button_state(choose_program,"daily",false)
  set_button_state(choose_program,"premium",true)
  main_chosen_program = "premium"
end

set_service = function(compare_service,name_service)
  if compare_service == "true" then 
    compare_service_new = "false"
    choose_add_services:Set("button_"..name_service.."_on.visible", "false") 
    choose_add_services:Set("button_"..name_service.."_off.visible", "true") 
    choose_add_services:Set(name_service.."_cost2.visible", "false")
    choose_add_services:Set(name_service.."_cost.visible", "true")
    choose_add_services:Set(name_service.."_cost_rub.src", "rub_red.png")
    service_var = "nothing"
    return compare_service_new,service_var
  else
    compare_service_new = "true" 
    choose_add_services:Set("button_"..name_service.."_on.visible", "true") 
    choose_add_services:Set("button_"..name_service.."_off.visible", "false")
    choose_add_services:Set(name_service.."_cost.visible", "false") 
    choose_add_services:Set(name_service.."_cost2.visible", "true")
    choose_add_services:Set(name_service.."_cost_rub.src", "rub.png")
    service_var = name_service
    return compare_service_new,service_var
  end
end

vacuum_cleaner_and_mats = function()
  compare_vacuum_cleaner_and_mats,vacuum_cleaner_and_mats_var = set_service(compare_vacuum_cleaner_and_mats,"vacuum_cleaner_and_mats")
end

interior_and_wheels = function()
  compare_interior_and_wheels,interior_and_wheels_var = set_service(compare_interior_and_wheels,"interior_and_wheels")
end

tire = function()
  compare_tire,tire_var = set_service(compare_tire,"tire")
end

disks = function()
  compare_disks,disks_var = set_service(compare_disks,"disks")
end

drying = function()
  compare_drying,drying_var = set_service(compare_drying,"drying")
end

show_payment_cash = function()
  payment:Set("payed_cost.value", cash_balance)
  payment:Set("to_pay_cost.value", summ_cost_var)
  payment:Display()
  smart_delay(100)
end

show_payment_electronical = function()
  payment:Set("payed_cost.value", electronical_balance)
  payment:Set("to_pay_cost.value", summ_cost_var)
  payment:Display()
  smart_delay(100)
end

show_choose_payment_method = function()
  debug_message("show_choose_payment_method")
  choose_payment_method:Set("to_pay_cost.value", summ_cost_var)
  choose_payment_method:Display()
  smart_delay(100)
end

show_choose_add_services = function()
  choose_add_services:Set("vacuum_cleaner_and_mats_cost.value", vacuum_cleaner_and_mats_cost)
  choose_add_services:Set("vacuum_cleaner_and_mats_cost2.value", vacuum_cleaner_and_mats_cost)
  choose_add_services:Set("interior_and_wheels_cost.value", interior_and_wheels_cost)
  choose_add_services:Set("interior_and_wheels_cost2.value", interior_and_wheels_cost)
  choose_add_services:Set("tire_cost.value", tire_cost)
  choose_add_services:Set("tire_cost2.value", tire_cost)
  choose_add_services:Set("disks_cost.value", disks_cost)
  choose_add_services:Set("disks_cost2.value", disks_cost)
  choose_add_services:Set("drying_cost.value", drying_cost)
  choose_add_services:Set("drying_cost2.value", drying_cost)  
  choose_add_services:Display()
  smart_delay(100)
end

show_choose_program = function()
  choose_program:Set("express_cost.value", express_cost)
  choose_program:Set("daily_cost.value", daily_cost)
  choose_program:Set("premium_cost.value", premium_cost)
  choose_program:Display()
  smart_delay(100)
end

show_contract1 = function()
  contract1:Display()
  smart_delay(100)
end

show_contract2 = function()
  contract2:Display()
  smart_delay(100)
end

show_contract3 = function()
  contract3:Display()
  smart_delay(100)
end

show_contract4 = function()
  contract4:Display()
  smart_delay(100)
end

show_contract5 = function()
  contract5:Display()
  smart_delay(100)
end

show_washing_started = function()
  washing_started:Display()
  smart_delay(100)
end

smart_delay = function(ms)
  hardware:SmartDelay(ms)
end

forget_pressed_key = function()
    key = get_key()
end

get_price = function(key)
    return registry:ValueInt(key)
end

get_key = function()
    return hardware:GetKey() 
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

debug_message = function(text)
    if is_debug then printMessage(text) end
end


update_balance = function()
  new_coins = hardware:GetCoins()
  new_banknotes = hardware:GetBanknotes()
  new_electronical = hardware:GetElectronical()
  new_service = hardware:GetService()

  cash_balance = cash_balance + new_coins
  cash_balance = cash_balance + new_banknotes
  cash_balance = cash_balance + new_electronical
  cash_balance = cash_balance + new_service

  balance = balance + new_coins
  balance = balance + new_banknotes
  balance = balance + new_electronical
  balance = balance + new_service
end 

set_defaults = function()


  balance = 0
  balance_seconds = 0
  kasse_balance = 0.0
  wait_card_mode_seconds = 40

  
  hascardreader = true
  is_transaction_started = false
  is_waiting_receipt = false

  
  --help variables
  short_delay_var = 1000 
  is_debug = false
  

  --WORKING SCREEN MODES
  mode_contract1 = 0
  mode_contract2 = 10
  mode_contract3= 20
  mode_contract4 = 30
  mode_contract5 = 40
  mode_choose_program = 70
  mode_choose_add_services = 80
  mode_choose_payment_method = 90
  mode_payment_cash = 100
  mode_payment_bank_card = 110
  mode_washing_started = 120

  --MAIN PROGRAMS SCREEN6
  function_express = 200
  function_daily = 210
  function_premium = 220  
  
  --main program prices variables
  express_cost = 200
  daily_cost = 250
  premium_cost = 350

  --main program state variable
  main_chosen_program = "nothing"
  --END MAIN PROGRAMS SCREEN6
  
  --SERVICE SCREEN7
  --add service program state variables
  vacuum_cleaner_and_mats_var = "nothing"
  interior_and_wheels_var = "nothing"
  tire_var = "nothing"
  disks_var = "nothing"
  drying_var = "nothing"

  --service variables for a swap white and red images
  compare_vacuum_cleaner_and_mats = "false"
  compare_interior_and_wheels = "false"
  compare_tire = "false"
  compare_disks = "false"
  compare_drying = "false"
  
  --variables for a choosing add service
  function_vacuum_cleaner_and_mats = 300
  function_interior_and_wheels = 310
  function_tire = 320
  function_disks = 330 
  function_drying = 340

  --service prices variables
  vacuum_cleaner_and_mats_cost = 50
  interior_and_wheels_cost = 250
  tire_cost = 50
  disks_cost = 150
  drying_cost = 50
  --END SERVICE SCREEN7    

  --PAYMENT METHOD SCREEN8
  --main summ var
  summ_cost_var = 0
  
  --payment method state variables
  payment_method_var = "nothing"

  --payment modes
  mode_cash = 400
  mode_bank_card = 410
  --END PAYMENT METHOD SCREEN8
   
  --PAYMENT SCREEN9
  cash_balance = 0
  electronical_balance = 0

  mode_finish = mode_contact1

  set_button_state(choose_program,"express",false)
  set_button_state(choose_program,"daily",false)
  set_button_state(choose_program,"premium",false)

  set_button_state(choose_add_services,"drying",false)
  set_button_state(choose_add_services,"disks",false)
  set_button_state(choose_add_services,"tire",false)
  set_button_state(choose_add_services,"interior_and_wheels",false)
  set_button_state(choose_add_services,"vacuum_cleaner_and_mats_on",false)

  --icon color of the ruble and digits
  compare_drying,drying_var = set_service("true","drying")
  compare_disks,disks_var = set_service("true","disks")
  compare_tire,tire_var = set_service("true","tire")
  compare_interior_and_wheels,interior_and_wheels_var = set_service("true","interior_and_wheels")
  compare_vacuum_cleaner_and_mats,vacuum_cleaner_and_mats_var = set_service("true","vacuum_cleaner_and_mats")

  --cash or bank card buttons
  set_button_state(choose_payment_method,"cash",false)
  set_button_state(choose_payment_method,"bank_card",false) 

  --button to pay(grey)
  payment:Set("button_to_pay_off.visible", "true")
  payment:Set("button_to_pay.visible", "false")
end





