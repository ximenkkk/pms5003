#include <pms.h>

////////////////////////////////////////

//
// Please uncomment #define PMS_DYNAMIC in pmsConfig.h file
//

PmsAltSerial pmsSerial;

#if defined PMS_DYNAMIC
pmsx::Pms* pms = nullptr;
#else
pmsx::Pms pms(&pmsSerial);
#endif

// * PMS5003 Pin 1 (black) : VCC +5V
// * PMS5003 Pin 2 (brown) : GND
// Important: pms5003 uses 3.3V logic.Use converters if required or make sure your Arduino board uses 3.3V logic too.
// * PMS5003 Pin 4 (blue) : Digital pin 9 (there is no choice, forced by AltSerial)
// * PMS5003 Pin 5 (green) : Digital pin 8 (there is no choice, forced by AltSerial)
// * Optional
//   * PMS5003 Pin 3 (white) : Digital pin 7 (can be changed or not connected at all)
//   * PMS5003 Pin 6 (violet) : Digital pin 6 (can be changed or not connected at all)

////////////////////////////////////////

void setup(void) {
    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("PMS5003");

#if defined PMS_DYNAMIC
    pms = new pmsx::Pms(&pmsSerial);
    if (!pms->initialized()) {
#else
    if (!pms->begin()) {
#endif
        Serial.println("Serial communication with PMS sensor failed");
        return;
    }

    pms->setPinReset(6);
    pms->setPinSleepMode(7);

    if (!pms->write(pmsx::PmsCmd::CMD_RESET)) {
        pms->write(pmsx::PmsCmd::CMD_SLEEP);
        pms->write(pmsx::PmsCmd::CMD_WAKEUP);
    }
    pms->write(pmsx::PmsCmd::CMD_MODE_PASSIVE);
    pms->write(pmsx::PmsCmd::CMD_READ_DATA);
    pms->waitForData(pmsx::Pms::TIMEOUT_PASSIVE, pmsx::PmsData::FRAME_SIZE);
    pmsx::PmsData data;
    auto status = pms->read(data);
    if (status != pmsx::PmsStatus::OK) {
        Serial.print("PMS sensor: ");
        Serial.println(status.getErrorMsg());
    }
    pms->write(pmsx::PmsCmd::CMD_MODE_ACTIVE);
    if (!pms->isWorking()) {
        Serial.println("PMS sensor failed");
    }

    Serial.print("Time of setup(): ");
    Serial.println(millis());
}

////////////////////////////////////////

void loop(void) {

    static auto lastRead = millis();
    pmsx::PmsData data;
    auto status = pms->read(data);

    switch (status) {
    case pmsx::PmsStatus::OK: {
        Serial.println("_________________");
        const auto newRead = millis();
        Serial.print("Wait time ");
        Serial.println(newRead - lastRead);
        lastRead = newRead;

        auto view = data.particles;
        for (auto i = 0; i < view.SIZE; ++i) {
            Serial.print(view[i]);
            Serial.print("\t");
            Serial.print(view.names[i]);
            Serial.print("\t");
            Serial.print(view.diameters[i]);
            Serial.print(" [");
            Serial.print(view.metrics[i]);
            Serial.print("] ");
            Serial.print(view.getLevel(i));
            Serial.println();
        }
        break;
    }
    case pmsx::PmsStatus::NO_DATA:
        break;
    default:
        Serial.print("!!! Pms error: ");
        Serial.println(status.getErrorMsg());
    }
}
