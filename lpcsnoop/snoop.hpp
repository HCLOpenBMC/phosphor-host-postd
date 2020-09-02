#pragma once

#include <gpiod.h>

#include <iostream>
#include <sdbusplus/asio/connection.hpp>
#include <xyz/openbmc_project/State/Boot/Raw/server.hpp>

/* The LPC snoop on port 80h is mapped to this dbus path. */
#define SNOOP_OBJECTPATH "/xyz/openbmc_project/state/boot/raw"
/* The LPC snoop on port 80h is mapped to this dbus service. */
#define SNOOP_BUSNAME "xyz.openbmc_project.State.Boot.Raw"

using message = sdbusplus::message::message;

static size_t totalHost; /* Number of host */

std::vector<gpiod_line*> leds;

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;
using PostInterface = sdbusplus::xyz::openbmc_project::State::Boot::server::Raw;
using PostObject = ServerObject<PostInterface>;

class PostReporter : public PostObject
{
  public:
    PostReporter(sdbusplus::bus::bus& bus, const char* objPath, bool defer) :
        PostObject(bus, objPath, defer)
    {
    }

    ~PostReporter()
    {
    }

    void readPostCode(uint16_t postcode, uint16_t host) override;
    int postCodeDisplay(uint8_t status, uint16_t host);
};

int readHostSelectionPos(int& pos)
{

    static boost::asio::io_service io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    auto value = conn->new_method_call("xyz.openbmc_project.Misc.Frontpanel",
                                       "/xyz/openbmc_project/misc/frontpanel",
                                       "xyz.openbmc_project.Misc.Frontpanel",
                                       "readHostPosition");
    try
    {
        message intMsg = conn->call(value);
        intMsg.read(pos);
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Failed to read the host position.\n";
        return -1;
    }

    std::cerr << "snoopd : Host position :" << pos << "\n";
    return 0;
}

int setGPIOOutput(void)
{
    int ret;
    gpiod_line* led;
    std::string gpio;
    int iteration;
    std::string gpioName = "LED_POST_CODE_";

    for (int iteration = 0; iteration < 8; iteration++)
    {
        gpio = gpioName + std::to_string(iteration);
        led = gpiod_line_find(gpio.c_str());
        if (!led)
        {
            std::cerr << "Failed to find the gpioname : " << gpio << std::endl;
            return -1;
        }

        leds.push_back(led);
        ret =
            gpiod_line_request_output(led, "LEDs", GPIOD_LINE_ACTIVE_STATE_LOW);
        if (ret < 0)
        {
            std::cerr << "Request gpio line as output failed.\n";
            return -1;
        }
    }

    return 0;
}

int writePostCode(gpiod_line* gpioLine, const char value)
{
    int ret;

    fprintf(stderr, "Display_PostCode: 0x%" PRIx8 "\n", value);

    ret = gpiod_line_set_value(gpioLine, value);
    if (ret < 0)
    {
        std::cerr << "Set line output failed.\n";
        return -1;
    }

    return 0;
}

// Display the given POST code using GPIO port
int PostReporter ::postCodeDisplay(uint8_t status, uint16_t host)
{
    int ret;
    int iteration;
    char value;
    int hostSWPos = 0;

    ret = readHostSelectionPos(hostSWPos);
    if (ret < 0)
    {
        std::cerr << "read host position failed.\n";
        return -1;
    }

    if (hostSWPos == host)
    {
        for (iteration = 0; iteration < 8; iteration++)
        {
            value = (status >> iteration) & 1;
            ret = writePostCode(leds[iteration], value);
            if (ret < 0)
            {
                std::cerr << "Failed to write the postcode in display\n";
                return -1;
            }
        }
    }

    return 0;
}

std::vector<std::unique_ptr<PostReporter>> reporters;

void PostReporter ::readPostCode(uint16_t postcode, uint16_t host)
{
    int ret;
    uint64_t code = 0;

    // Display postcode received from IPMB
    ret = postCodeDisplay(postcode, host);
    if (ret < 0)
    {
        std::cerr << "error in display the postcode\n";
    }

    code = le64toh(postcode);

    if ((host + 1) <= totalHost)
    {
        // HACK: Always send property changed signal even for the same code
        // since we are single threaded, external users will never see the
        // first value.
        reporters[host]->value(~code, true);
        reporters[host]->value(code);
    }
    else
        fprintf(stderr, "Invalid Host :%d\n", totalHost);

    // read depends on old data being cleared since it doens't always read
    // the full code size
    code = 0;
}
