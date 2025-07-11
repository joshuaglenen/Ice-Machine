# Ice-Machine
This project documents the reverse engineering of a failed countertop ice machine using an Arduino Uno R3 (running C++) and a selection of salvaged electronics. The system was restored to full functionality through hands-on troubleshooting, firmware development, and hardware prototyping.

### Diagnose Problem

The family ice machine had entered a fail state in which it continuously emptied the tray and then shut down. I carefully disassembled the unit and inspected each component and sensor. All major components appeared operational, though the water flow sensor required cleaning and the infrared phototransistor had to be replaced. Despite reassembly and sensor repairs, the machine remained stuck in the same state. Upon inspecting the original PCB, I found no visible faults, but due to the lack of functionality and difficulty in debugging the proprietary controller, I made the decision to reconstruct the logic board from scratch using my own components and code.

### Salvage Working Modules

I mapped out the thermal cycle and mechanical process of the original machine and modeled the system as a state machine. The AC synchronous motor rotates the tray between “load” and “unload” positions, and a small DC pump fills the tray with water. After freezing, the tray rotates again to eject the formed ice into a storage bin. Thermodynamically, the system operates by compressing refrigerant and passing it through a muffler and condenser where a heatsink and fan rapidly cool the gas into a liquid. This liquid is then cycled through the evaporator fingers submerged in water. As the refrigerant absorbs heat, ice crystals form over the fingers. After a 15-minute freezing cycle, the solenoid opens, allowing hot refrigerant gas to rush into the evaporator, warming the metal fingers and releasing the ice.

### Circuit Design 

The circuit diagrams were developed through iterative design to accommodate the components I had on hand and to reuse as much of the existing machine wiring as possible. The completed system is built around an Arduino Uno R3, which manages all input monitoring, state logic, and output control. For AC components, relays are used to switch 120V lines (live wire connected to NO terminal; neutral wired directly to the component). For DC devices such as the water pump and LED indicators, NPN MOSFETs are used to sink current to ground from a 9V supply.

The full wiring diagram will be included below as Diagram 1. A schematic of the original control board is also included as Diagram 2 for comparison.

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



### State Machine Implementation

The machine operates as a four-state cycle:

    State 1: Wake up and unload the tray of any residual water or ice. The IR sensor is checked for signs of ice accumulation in the bin; if ice is detected, the system enters sleep mode to prevent overproduction.

    State 2: The tray is moved to the load position, water is pumped into the tray, and the IR system also detects whether newly formed ice was released. If water flow is not detected during this process, the system enters sleep mode.

    State 3: The compressor and fan are activated to begin the freezing cycle. The system holds in this state for a fixed time.

    State 4: The compressor and fan are deactivated. The solenoid is opened to allow hot refrigerant into the evaporator, causing the ice to release.

An interrupt-driven push button can be used to toggle the system into or out of sleep mode. During sleep, the machine is entirely powered down except for a power LED, which flashes once per second as an indicator. While active, the power LED remains solid.

### Firmware Development 

The firmware was written in Arduino C++ and structured around the state machine described above. It handles all I/O reads and writes, sensor thresholds, timing delays, interrupt routines, and fault logic. The code resides in main.ino.

### Component Testing

Each module was tested in isolation using a known power supply and confirmed for correct behavior. Sensor outputs were measured using a test sketch to determine their voltage ranges and trigger points. The IR sensor, flow sensor, and limit switches were all evaluated and tuned before being included in the full system.

### Prototyping and Assembly

The Arduino Uno, relay assembly, perfboard shield, and DC power supply were assembled into a functioning prototype. All wiring was verified with a multimeter for continuity, correct voltage levels, and isolation between AC and DC subsystems. The prototype was field-tested before installation, and later secured with hot glue to reduce the risk of shorts or mechanical interference.


![IMG_0569](https://github.com/user-attachments/assets/26829434-1e3f-4a06-9314-3e113714766b)

Fig 7: Systems wired and tested

### Final Testing

Cycle timing was adjusted based on performance testing. Water flow and ice detection thresholds were tuned, and various delays were optimized for reliable and timely ice production. The system was validated over several full cycles, and usability was improved with the addition of a push-button power control and LED feedback indicator.

![IMG_0577](https://github.com/user-attachments/assets/b92e91a7-7148-43be-962a-42cde7ad6ed6)

Fig 8: Ice

### Deployment 

The completed machine was reassembled and returned to service. It has been operating reliably since the rebuild. A custom PCB and complete wiring diagram have been constructed in KiCad and will be added below

<img width="3507" height="2480" alt="circuit" src="https://github.com/user-attachments/assets/c43c284b-309e-4617-b41c-8014744bae3c" />

Diagram 1: Circuit of Embedded System

<img width="666" height="519" alt="Capture" src="https://github.com/user-attachments/assets/b3576ca5-78cd-4a53-90be-dfc7369eb402" />

Diagram 2: 3D PCB Model Simulated in KiCAD
