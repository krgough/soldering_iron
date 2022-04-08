# soldering_iron

Schematic, PCB and Firmware for a temperature controlled soldering iron.   
Based on >> https://www.instructables.com/DIY-Digital-Soldering-Station/?utm_source=newsletter&utm_medium=email

Design Notes:
Soldering Iron has a 5 ohm heater coil and a PTC temperature sensor.  So we switch our source voltage (24v or less) via
a mosfet through the heater to operate the iron.  We measure the tip temperature via the PTC.

Wanted Temp Range = 500-24'C = 475 degree steps
10 bit ADC resolution = 5 v* (2^10 - 1) = 4.887mV/div

PTC Values are approx:
24'C = 50R
500'C = 215R

For the PTC we use a potential divider of 470R + PTC(approx 50R at 24'C).   This results in a voltage range of around
0.48v - 1.49v = 1v = 207 ADC steps.   Each ADC step is therefore equivalent to more than 1'C, so we would have missing
temperature values.

To overcome that problem we use an opamp with a gain of 2.22
G = Rf/Ri + 1 = 3300/2700 + 1 = 2.22

24'C    = 0.48 * 2.22 = 1.06v
500'C = 1.57 * 2.22 = 3.49v

This gives a range of 2.25v or 497 ADC steps.

Note that Max Vout from LM358 is around 3v5 when running from 5v, so it's hard to get any more without introducing a level
shift as well.  I did experiment with using the spare opamp as a unity gain buffer to put a non-zero voltage on the gain
setting resistor that would normally be connected to ground - this seemed to create odd behaviour (output was not linear)
so I abandoned that as the simple solution above seems to be close enough.


Display
We use a 16x2 LCD with a I2C interface.  In my case I'm using an Adafruit module that was originally purchased for use with an rPi.

Spare OpAmp
Terminated the device as a unity gain buffer with input set to mid rail.

Adjustable SMPS
We power the device with 18-24v and then use the SMPS to regulate down to 5v for the control circuits and display.  Since the
SMPS is adjustable I put a jumper in the output so that I can adjust the voltage with the rest of the circuit disconnected.

Firmware
- We average the readings to try to reduce any noise for the control loop.
- Display refresh - we limit the refresh rate to once a second to stop and annoying flickering 
- Hysteresis - We have implemented some temperature hysteresis on the control loop to prevent rapid switching - it may not be required but it seems to be sensible to prevent any rapid oscillations.

Protection/Safety Considerations (To be done - not yet implemented)
The Iron can reach 500'C or higher when left on continuously.  For safety I think it's worth having a couple of features...
- If the Arduino crashes with the MOSFET on then Iron can overheat.  Mitigation is to implement watchdog in Arduino so that a WD Reset will occur if the device locks out for some reason.  This is not entirely ideal - We could also have some external circuit like a monostable, so that we have to keep sending pulses to the monostable to enable the MOSFET, but this would require additional circuitry.
- Limit the max use time to x minutes.  Use a timer to turn the Iron to standby after a given time.  User needs to adjust the setpoint to make it come back on.  Iron shall start up in standby so that user interaction is required before the heater is turned on.   Suggest this is done by creating a new state variable "bool standby = 0".   Standby is disabled by a change of > 5'C on the setpoint. 
