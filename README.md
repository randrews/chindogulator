# Chindogulator
A Chindogu calculator for your computer

![chindogulator.jpg]

## What is this?
This is an RPN calculator that uses a mechanical keypad for input, and USB keyboard for output: you type in numbers, it echoes them to your PC; you press a function key and it backspaces that number and types in the result.

## How do I use it?
Chindogulator is a simple RPN calculator with a stack four elements tall, although visually you can only see the top element. You can type numbers into the first cell, press enter to store that number, then type in another number followed by an operation. This will pop both numbers from the stack and push the result (printing it to the keyboard).

For example: press `2`, `3`, `enter`, `6`, and `+`. This will enter the number 23, push it on to the stack, then enter the number six, then add them together and print the result (29).

Functions exist for the basic arithmetic operations, trigonometry, and taking a natural logarithm.

## This is stupid, I can't use it unless I'm already at a computer
See the definition of "[Chindogu](https://en.wikipedia.org/wiki/Chind%C5%8Dgu)"

## How do I make one?
In the `hardware` folder are Eagle and Gerber files for two PCBs. You can either fabricate these as you want, or perfboard an equivalent circuit. For the `keys` PCB you will want 25 Cherry switches (or equivalent) and 25 diodes; for the `controller` PCB you'll need an atmega32u4 chip, the resistors and 1uF capacitor marked on the board, a 16 MHz crystal and a mini-USB plug. Or the controller can be replaced with a Teensy 2.0.

Then you'll need keycaps: I used relegendable keycaps from Signature Plastics, with the legends in the `keycaps` folder printed and cut out with scissors. You'll probably want to scale it down for printing.

Finally the firmware for the microcontroller is written with the Arduino toolchain and provided in the `chindogulator` folder.