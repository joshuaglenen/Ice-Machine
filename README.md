# Ice-Machine
Reverse Engineered a broken Ice machine to work with an old UNO R3 running C++ and various scrap electronics.

Diagnose Problem - Family ice machine was stuck in a fail state where it would empty the tray and shutdown. I carfully opened the machine and examined each of the sensors and components. All components were working however the water flow sensor needed to be cleaned and the infrared phototransitor needed to be replaced. Upon reassmbling, the ststem continued in a failstate. I examined the pcb and found no fault so i decided to reconstruct the pcb with my own components and code.

Salvage Working Modules- All modules were working after i repaired the sensors. I worked out how each component worked during the normal operation of ice making and reused everything except the MCU, optoisolated relays, a 9vdc psu and various small components.

Draw System Diagrams - I reconstructed the cooling and warming process into a state machine. The AC motor moved the tray location, the dc motor pumped water into the tray, the movement of the tray pushed the ice out and into a recepticle. The compressor pumped high pressure hot refridgerant into a muffler then into the condensor where a heat sink and cooling fan reapidly cooled the gas into a liquid. This liquid was pumped into the evaporator fingers where it made contact with the water in the tray. Ice crystals would form and grow over a period of fifteen minutes. The liquid would heat up and reenter the compressor at low pressure. After the period ended, a solenoid would activate forcing hot gas into the evaporator releasing the ice.

![IMG_04BE4556-BE88-4E1F-9EAD-9990F7E11C73](https://github.com/user-attachments/assets/f8083d9c-000f-47a8-bda1-18b66848e5c9)
Fig 1: Limit switches that indicate the synchronous motor has reached the load or unload position

![IMG_6F6667B0-8BAD-4ADE-BD0C-B2546FD11910](https://github.com/user-attachments/assets/478b5e5f-1121-4f94-bb20-95147a53d363)
Fig 2: Compressor and solenoid

![IMG_7E9F9666-A5EF-4799-809D-BFA2EDB27531](https://github.com/user-attachments/assets/92e7ec20-3e97-402e-89f5-3fd8525db37f)
Fig 3: Synchronous motor

![IMG_0779B9D5-F53D-4FA0-A4C4-8B1841A305D5](https://github.com/user-attachments/assets/e9d83ddc-b2cf-4390-92cb-c093869ce0d4)
Fig 4: Tray mechanism with IR LED and phototransistor

![IMG_0578](https://github.com/user-attachments/assets/a04a6115-76ca-456a-aa0a-39a062679e80)
Fig 5: Evaporator with cooling fingers in action

![IMG_0538](https://github.com/user-attachments/assets/e5522834-e1b3-46f2-ab96-7737070ccbb1)
Fig 6: Water sensor

Draw Circuit Diagrams - The circuit diagrams were drawn and iterated to use availible components and as much of the original machine as possible. The complete wiring diagram can be seen in diagram 1 with the unused pcb sketch in diagram 2. An UNO R3 was used as it was freely availible and would be more than capable for the task of reading inputs, controlling state machine logic, and enableing outputs. Relays were used to send 120VAC live lines to the no ports wired to the components while neutral was wired to the components directly. Npn mosfets were used to control the current flow from 9VDC to ground.

Create State Machine Logic - State 1 wakes the machine up and empties the tray of ice or water. It then checks if the voltage spikes indicating the ice tray is full and if so enters sleep mode to stop overproducing ice. State 2 loads the tray with water and also pushes ice out into the ice tray. While water is being pumped, the water sensor tests if the voltage drops and if not it enters sleep mode. State 3 enables the cooling process with the compressor and cooling fan. The system waits for a set period. State 4 disables the compressor and fan and enables the solenoid. This causes hot gas to rush into the evaporator causing the ice to release. After a set period the system resets. Sleep mode can be enetered or exited any time through an ISR operated push button. During sleep mode, the system powers down and an indicator LED which is normally always on flashes once per second.

Write Firmware - The code in main.ino controls the entire system with a state machine using the UNO R3. The system was programmed in Arduino.

Test Components Individually - Each component was tested using a know power source corresponding to the specific module. The outputs for the sensors were read by a test program on the UNO R3 to determine ideal operating range. 

Assemble Prototype - The protoboard shield, UNO r3 dev kit, relay assembly, and dc psu were assembled and the wires were connected together. Wiring was double checked and tested with a multimeter to ensure continuity and proper voltage levels.

![IMG_0569](https://github.com/user-attachments/assets/26829434-1e3f-4a06-9314-3e113714766b)
Fig 7: Systems wired and tested

Test Final System - During testing, the adjustable values for the sensor readings, the time to freeze, the time to thaw, and the time to fill the water tray were adjusted to produce optimal and expediant ice. The program was debugged and a sleep/wake feature with a power button was implemented to make the system more usable.

![IMG_0577](https://github.com/user-attachments/assets/b92e91a7-7148-43be-962a-42cde7ad6ed6)
Fig 8: Ice

Deploy - Ice machine was put back in its original place and has been making ice ever since. I have reconstructed the wiring diagram and pcb in kicad and implemented them below.

Diagram 1:

Diagram 2:
