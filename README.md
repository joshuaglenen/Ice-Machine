# Ice-Machine
Reverse Engineered a broken Ice machine to work with an old UNO R3 running C++ and various scrap electronics.

Steps:

Diagnose Problem - Family ice machine was stuck in a fail state where it would empty the tray and shutdown. I carfully opened the machine and examined each of the sensors and components. All components were working however the water flow sensor needed to be cleaned and the infrared phototransitor needed to be replaced. Upon reassmbling, the ststem continued in a failstate. I examined the pcb and found no fault so i decided to reconstruct the pcb with my own components and code.

Salvage Working Modules- All modules were working after i repaired the sensors. I worked out how each component worked during the normal operation of ice making and reused everything except the MCU, optoisolated relays, a 9vdc psu and various small components.

Draw System Diagrams - I reconstructed the cooling and warming process into a state machine. First the machine would wake up, then unload any ice in the tray and then move to the poad position. The movement to the load position alsi pushed the ice into a recepticle. The infrared sensor detected the level of the ice recepticle and would shut down if the input went high. Water was then pumped into the tray and the water sensor detected the presence of water - if the input was high, the system would shut down.

The compressor and cooling fab were then activated and the cool high pressure refridgerant was pumped into the cooling fingers in the tray where ice formed. The coolant then entered the

Draw Circuit Diagrams -

Create State Machine Logic -

Write Firmware -

Test Components Individually -

Assemble Prototype - 

Test Final System -

Deploy - 
