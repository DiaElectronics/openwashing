# functions available

hardware:TurnProgram(program_num) //pass -1 as relay_num to turn all lights
hardware:TurnActivator(relay_num, is_on) //pass -1 as relay_num to turn all relays
hardware:GetCoins()
hardware:GetBanknotes()
hardware:GetKey()
screen_name:Display() to display a screen;



hardware:TurnLight(1, animation.stop)
hardware:TurnLight(1, animation.one_button)
hardware:TurnLight(1, animation.idle)
hardware:TurnLight(1, animation.intense)
