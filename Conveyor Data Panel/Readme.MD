This is a project POC made for an conveyor belt data panel for a leather factory.
The panel consists of a parallel 128x64 LCD display connected to an MCP23017 I2C module and powered by an ATMEGA328P
The IO ports are exposed for connecting 2 Rotary Encoders and an IR object counter.

The dualtacho.ino uses 2 non contact proximity sensors instead of 2 rotary encoders
Th RPM.ino uses 2 rotary encoders.
A non volatile ram module is connected to prevent data loss during power down.

Refer to the schematics for circuit diagram. 
