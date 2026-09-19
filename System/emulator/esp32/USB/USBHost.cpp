#include "USBHost.h"

class USBHostInt : public USBHost {
public:
    USBHostInt() {
    }

    void init() override {
    }

    void keyboardSetLeds(uint8_t leds) override {
    }
};

USBHost *getUSBHost() {
    static USBHostInt obj;
    return &obj;
}
