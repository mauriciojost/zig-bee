# README

This is a project for the **Ubinet's Master, Polytech'Nice, France**, subject **WPAN / WLAN**, year **2010**.

It implements a simplistic **network protocol** (IP-like) on top of **ZigBee datalink** protocol. 

Computers connect to ZigBee modules via serial port. ZibBee modules provide **datalink**, which will be exploited sending commands from the PC. The PC will implement the network protocol with the help of `routing tables`, and will be able to communicate among themselves. Execution takes place on several PCs-with-ZigBee-modules simultaneously.

## EXECUTE

To make this project run: 

- For each node (computer + Zibgee module) open the provided project file `aodv.dev` with the IDE `DevC++` (version 4.9.9.2 was used during development). 
- Then build the project and execute it. 

### INSTRUCTIONS

The system gets instructions from the user via keyboard commands:

- Press `c` to **c**hange destination address and message
- Press `w` to sho**w** routing table & rebroadcast table
- Press `p` to **p**roceed
- Press `r` to **r**ead incoming messages
- Press `s` to **s**end data message to destination (destination address must be set before executing this command)
- Press `h` to send a **h**ello message
- Press `t` to **t**est sending any raw message
- Press `q` to **q**uit the application

### NOTES

- Each message must be sent from one PC, and READ from another (the application is intended to be used in an academic way). 

- If the ZigBee module is in command line mode, it will not receive messages, so it's pointless have multiple threads when the own device behaves as a sequential device. 

## CONTRIBUTORS

- MADZUMAR Somnath

- JOST Mauricio

- SEIFU Sentayehu
