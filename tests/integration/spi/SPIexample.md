# SPI Example

This example code demonstrates SPI slave-master communication between an Arduino Mega (master) and an Arduino Nano (slave) through an LED connected to each device. The Mega (master) sends its LED state to the Nano (slave) via SPI, and the Nano mirrors this state to its own LED while sending its current state back to the Mega. The Mega then toggles its LED state to be the opposite of the received state from the Nano, which causes a visible LED toggling effect on both devices.

Watch [this video](https://youtu.be/0nVNwozXsIc?si=dTtKkJWaDG93pys8) to understand how SPI communcation works.

## Wiring

> Make sure to connect both devices and the LEDs to a common ground.

### SPI Connections

- Nano D10 to Mega D53
- Nano D11 to Mega D51
- Nano D12 to Mega D50
- Nano D13 to Mega D52

### Arduino Nano 

- D2 to a 220 ohm resistor which is connected to the positive end of an LED

### Arduino Mega

- D47 to a 220 ohm resistor which is connected to the positive end of an LED