# Flatpack2

Arduino Library for communicating with Eltek Flatpack2 power supplies.

I found multiple pieces of code for communicating with Flatpack2 power supplies
but I wanted a library so my program code would be more clean.

### Features
- Automaticly login and assign ID to powersupplies on CAN bus
- Store last status in an object
- Call an callback on status request
- Send a broadcast to change voltage & current

### TODO
- Test with other revisions and multiple devices on bus
- Test current limit
- Add bootup voltage
- Cleanup receive code

### Thanks to
- https://github.com/the6p4c/Flatpack2
- https://github.com/Lennart-O/ESP32_ELTEK_FLATPACK
- https://github.com/neg2led/flatpack2s2
- https://vk4ghz.com/product/eltek-flatpack2-24-48v-touchscreen-controller-kit/
- https://vk4ghz.com/wordpress/wp-content/uploads/Eltek-Flatpack2-Controller-User-Guide-29-07-2020.pdf
- https://electricmotorcycleforum.com/boards/index.php?topic=6405.135;wap2
- https://www.elithion.com/lithiumate/php/eltek.php

### Dependencies
- https://github.com/sandeepmistry/arduino-CAN
- https://github.com/hideakitai/DebugLog

### Status
Code is very alpha currently.

### Compatibility

| Description                   | Part number | Revision | Status | Set voltage | Set current | Bootup voltage |
| ----------------------------- | ----------- | -------- | ------ | ----------- |------------ | -------------- |
| FP2 48V 3KW FRONT END RECT HE | 380875      | 1.10     | YES    | YES         | UNTESTED    | UNTESTED       |

