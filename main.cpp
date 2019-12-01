/*
 * File:   main.cpp
 * Author: Radu Grecu, Dominik Kovacs
 *
 * Created on November 22, 2019, 10:30 AM
 */

using namespace std;

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <cmath>
#include "HIH8120.h"


// initialize LDR
void ldr_init()
{
    fstream fs;
    fs.open("/sys/bus/iio/devices/iio:device0/scan_elements/in_voltage5_en");
    fs << "1";
    fs.close();

    fs.open("/sys/bus/iio/devices/iio:device0/buffer/enable");
    fs << "1";
    fs.close();
}

// initialize LEDs
void led_init()
{
    char config[10];
    FILE *fp;

    // configure port to PWM
    fp = popen("config-pin -q P9_21", "r");
    fscanf(fp, "%*s %*s %s", config);
    fclose(fp);

    // override pin mode
    if (strcmp(config, "pwm"))
    {
        system("config-pin -a P9_21 pwm");
    }

    // configure PWM channel
    fstream fs;
    fs.open("/sys/class/pwm/pwmchip1/export");
    fs << "1";
    fs.close();
}

// initialize servo motor
void servo_init()
{
    char config[10];
    FILE *fp;

    // configure port to PWM
    fp = popen("config-pin -q P9_14", "r");
    fscanf(fp, "%*s %*s %s", config);
    fclose(fp);

    // override pin mode
    if (strcmp(config, "pwm"))
    {
        system("config-pin -a P9_14 pwm");
    }

    // configure PWM channel
    fstream fs;
    fs.open("/sys/class/pwm/pwmchip3/export");
    fs << "0";
    fs.close();

}

// initialize heater "LED"
void heater_init()
{
    // set gpio pin direction
    fstream fs;
    fs.open("/sys/class/gpio/gpio48/direction");
    fs << "out";
    fs.close();
}

// control light intensity
// input: intensity [0 - 100]
void set_light(unsigned int intensity)
{
    fstream fs;

    fs.open("/sys/class/pwm/pwmchip1/pwm-1:1/period");
    fs << "20000000";
    fs.close();

    fs.open("/sys/class/pwm/pwmchip1/pwm-1:1/duty_cycle");
    fs << to_string((int)(20000000 * (intensity / 100.0)));
    fs.close();

    fs.open("/sys/class/pwm/pwmchip1/pwm-1:1/enable");
    fs << "1";
    fs.close();
}

// open the box
void tray_open()
{
    fstream fs;

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/period");
    fs << "20000000";
    fs.close();

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/duty_cycle");
    fs << "1425000";
    fs.close();

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/enable");
    fs << "1";
    fs.close();

    // sleep for 1 seconds so servo has enough time to adjust (in microseconds)
    usleep(1 * 1000 * 1000);

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/enable");
    fs << "0";
    fs.close();
}

// close the box
void tray_close()
{
    fstream fs;

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/period");
    fs << "20000000";
    fs.close();

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/duty_cycle");
    fs << "545000";
    fs.close();

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/enable");
    fs << "1";
    fs.close();

    // sleep for 1 seconds so servo has enough time to adjust (in microseconds)
    usleep(1 * 1000 * 1000);

    fs.open("/sys/class/pwm/pwmchip3/pwm-3:0/enable");
    fs << "0";
    fs.close();
}

int main(int argc, char *argv[]) {

    bool open = true,
         manual_override = false,
         manual_heater = false,
         manual_tray = false,
         reading_override = false,
         heater_status = false;

	short reading = 0, luminosity = 0;
	fstream fs_heater, fs_manual_heater, fs_manual_override, fs_manual_tray;
    string line = "";

    exploringBB::HIH8120 sensor = exploringBB::HIH8120(2, 0x27);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////// ldr_init();
    led_init();
    servo_init();
    heater_init();

	while (1)
	{
        // store current operation mode -> manual/automated
        fs_manual_override.open("/sys/class/gpio/gpio45/value");
        fs_manual_override >> reading_override;

        // if there's a change in operation mode, store current output values to the override pins
        if (reading_override != manual_override){
            manual_override = reading_override;

            fs_heater.open("/sys/class/gpio/gpio48/value");
            fs_heater >> heater_status;

            fs_manual_heater.open("/sys/class/gpio/gpio67/value");
            fs_manual_heater << heater_status;
            fs_manual_heater.close();
            fs_heater.close();

            fs_manual_tray.open("/sys/class/gpio/gpio66/value");
            fs_manual_tray << to_string((int) open);
            fs_manual_tray.close();
        }
        fs_manual_override.close();


        if (manual_override) {
            ///////  Manual Mode  ///////

            // read heater mode [ on / off ]
            fs_manual_heater.open("/sys/class/gpio/gpio67/value");
            fs_manual_heater >> manual_heater;
            fs_manual_heater.close();

            // read tray mode [ open / closed]
            fs_manual_tray.open("/sys/class/gpio/gpio66/value");
            fs_manual_tray >> manual_tray;
            fs_manual_tray.close();

            // set heater mode
            fs_heater.open("/sys/class/gpio/gpio48/value");
            fs_heater << to_string(manual_heater);
            fs_heater.close();

            // set tray mode
            // tray operation will only occur if the new mode is different from the current one
            if (open && !manual_tray) {
                tray_close();
                open = false;
            } else if (!open && manual_tray){
                tray_open();
                open = true;
            }

        } else {
            ///////  Automated Mode  ///////

            // read LDR value
            ifstream fs_ldr("/sys/bus/iio/devices/iio:device0/in_voltage5_raw");

            // parse ADC value and store it
            getline(fs_ldr, line);
            reading = stoul(line);

            // change LED intensity only when there's a significant difference
            if (abs(reading - luminosity) > 50)
            {
                luminosity = reading;
                set_light(100 - luminosity/pow(2, 12) * 100);
            }
            fs_ldr.close();

            // read Humidity & Temperature
            sensor.read_sensor();

            // control heater and tray depending on read temperature
            if ((sensor.temperature > 28.0 && !open){
                tray_open();
                fs_heater.open("/sys/class/gpio/gpio48/value");
                heater_status = 0;
                fs_heater << to_string(heater_status);
                fs_heater.close();
                open = true;
            } else if (sensor.temperature < 28.0 && open){
                tray_close();
                fs_heater.open("/sys/class/gpio/gpio48/value");
                heater_status = 1;
                fs_heater << to_string(heater_status);
                fs_heater.close();
                open = false;
            }

            // 1 second interval
            usleep(1 * 1000 * 1000);
        }

	}

	return 0;
}
