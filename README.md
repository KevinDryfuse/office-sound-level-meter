# roar-of-the-crowd
Wall "art" that assesses noise levels in my office and provides a colorful visual representation of those levels.

## Expected Functionality

* Sound Level Meter to measure noise levels
* Screen to:
    * show current sound level
    * show calibration percentage
    * show adjusted min and max db levels on the display
* 256 LED Matrix to visually display sound level
* 4x4 membrane to:
    * calibrate.
    * Adjust min and max db levels on the display
* Ability to store settings so they don't reset (EEPROM capable board)

## Prototype Layout
![photo of prototype](/images/photo.JPEG)
![image of prototype](/images/diagram.PNG)

## Parts List

* [Arduino Mega](https://www.amazon.com/ELEGOO-ATmega2560-ATMEGA16U2-Compatible-Arduino/dp/B091DSB8HY)
* [Breadboard](https://www.amazon.com/Breadboards-Solderless-Breadboard-Distribution-Connecting/dp/B07DL13RZH)
* [Gravity Analog Sound Sensor](https://wiki.dfrobot.com/Gravity__Analog_Sound_Level_Meter_SKU_SEN0232) 
* [Addressable LED Matrix (32x8)](https://www.btf-lighting.com/products/ws2812b-panel-screen-8-8-16-16-8-32-pixel-256-pixels-digital-flexible-led-programmed-individually-addressable-full-color-dc5v?variant=20203594612836)
* [Membrane Switch Module (4x4)](https://tinkersphere.com/buttons-switches/762-adhesive-keypad-membrane-matrix-3x4.html)
* [1000µF electrolytic capacitor](https://www.amazon.com/gp/product/B07Y3F2TQ7) 
* [LCD Panel (20x4 - L2C)](https://www.amazon.com/gp/product/B01GPUMP9C/)
* 220Ω resistor
* [5V 15A AC to DC power supply](https://www.amazon.com/ALITOVE-Transformer-Converter-5-5x2-1mm-100V-240V/dp/B01LXN7MN3)
* [Raspberry Pi/Arduino/Breadboard (RAB) holder](https://www.amazon.com/SunFounder-Holder-Raspberry-Breadboard-Arduino/dp/B081167YZM)
* [A bunch of 22 AWG hook-up wire](https://www.amazon.com/TUOFENG-Wire-Solid-different-colored-spools/dp/B07TX6BX47/)

## ToDo
* Saving the color scheme would come in handy, as I prefer blue but it defaults to green.
* Labeling the buttons would probably be nice ... I forget what each one does.
* The LCD screen uses a ton of power.  I've configured it to be able to turn off the backlight, but there might be better options for the amount of draw it requires especially while the matrix is fully lit.