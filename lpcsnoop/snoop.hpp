#pragma once

#include <gpiod.h>
#include <unistd.h>

#include <boost/process/io.hpp>
#include <iostream>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/State/Boot/Raw/server.hpp>


#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/asio/sd_event.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/timer.hpp>


#include <variant>

/* The LPC snoop on port 80h is mapped to this dbus path. */
#define SNOOP_OBJECTPATH "/xyz/openbmc_project/state/boot/raw"
/* The LPC snoop on port 80h is mapped to this dbus service. */
#define SNOOP_BUSNAME "xyz.openbmc_project.State.Boot.Raw"

#define BIT(value, index) ((value >> index) & 1)

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;
using PostInterface = sdbusplus::xyz::openbmc_project::State::Boot::server::Raw;
using PostObject = ServerObject<PostInterface>;

using variant = std::variant<int, std::string>;

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

    void readPostcode(uint16_t postcode, uint16_t host) override;
    int PostCodeDisplay(uint8_t status);
};

PostReporter* ptrReporter;
PostReporter* ptrReporter1;
PostReporter* ptrReporter2;
PostReporter* ptrReporter3;

gpiod_line* led0;
gpiod_line* led1;
gpiod_line* led2;
gpiod_line* led3;
gpiod_line* led4;
gpiod_line* led5;
gpiod_line* led6;
gpiod_line* led7;


bool setGPIOOutput(void)
{

    int ret;

    led0 = gpiod_line_find("LED_POST_CODE_0");
    if (!led0)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led0, "LED0", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }

    led1 = gpiod_line_find("LED_POST_CODE_1");
    if (!led1)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led1, "LED1", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }

    led2 = gpiod_line_find("LED_POST_CODE_2");
    if (!led2)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led2, "LED2", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }
    led3 = gpiod_line_find("LED_POST_CODE_3");
    if (!led3)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led3, "LED3", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }

    led4 = gpiod_line_find("LED_POST_CODE_4");
    if (!led4)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led4, "LED4", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }
    led5 = gpiod_line_find("LED_POST_CODE_5");
    if (!led5)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led5, "LED5", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }

    led6 = gpiod_line_find("LED_POST_CODE_6");
    if (!led6)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led6, "LED6", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }
    led7 = gpiod_line_find("LED_POST_CODE_7");
    if (!led7)
    {
        std::cerr << "Failed to find the line.\n";
        return false;
    }

    ret = gpiod_line_request_output(led7, "LED7", GPIOD_LINE_ACTIVE_STATE_LOW);
    if (ret < 0)
    {
        std::cerr << "Request line as output failed.\n";
        return false;
    }

    return true;
}

bool WritePostCode(gpiod_line* gpioLine,
                                          const char value)
{
    int ret;
    int hostSWPos = 1;

    fprintf(stderr, "Display_PostCode: 0x%" PRIx8 "\n", value);

    ret = gpiod_line_set_value(gpioLine, value);
    if (ret < 0)
    {
        std::cerr << "Set line output failed.\n";
        return false;
    }

    return true;
}

// Display the given POST code using GPIO port
int PostReporter ::PostCodeDisplay(uint8_t status)
{
    char value;

    value = (BIT(status, 1)) ? 1 : 0;
    if (!WritePostCode(led0, value))
    {
        return -1;
    }

    value = (BIT(status, 2)) ? 1 : 0;
    if (!WritePostCode(led1, value))
    {
        return -1;
    }

    value = (BIT(status, 3)) ? 1 : 0;
    if (!WritePostCode(led2, value))
    {
        return -1;
    }
    value = (BIT(status, 4)) ? 1 : 0;
    if (!WritePostCode(led3, value))
    {
        return -1;
    }

    value = (BIT(status, 5)) ? 1 : 0;
    if (!WritePostCode(led4, value))
    {
        return -1;
    }

    value = (BIT(status, 6)) ? 1 : 0;
    if (!WritePostCode(led5, value))
    {
        return -1;
    }
    value = (BIT(status, 7)) ? 1 : 0;
    if (!WritePostCode(led6, value))
    {
        return -1;
    }

    value = (BIT(status, 8)) ? 1 : 0;
    if (!WritePostCode(led7, value))
    {
        return -1;
    }

    return 0;
}

void PostReporter ::readPostcode(uint16_t postcode, uint16_t host)
{
    uint64_t code = 0;

    // Display postcode received from IPMB
    PostCodeDisplay(postcode);

    code = le64toh(postcode);
    fprintf(stderr, "Code: 0x%" PRIx64 "\n", code);

    printf("postcode = %d host = %d\n", (int)postcode, (int)host);
    std::cout.flush();

    switch (host)
    {
        case 0:
            // HACK: Always send property changed signal even for the same code
            // since we are single threaded, external users will never see the
            // first value.
            ptrReporter->value(~code, true);
            ptrReporter->value(code);
            break;

        case 1:
            ptrReporter1->value(~code, true);
            ptrReporter1->value(code);
            break;

        case 2:
            ptrReporter2->value(~code, true);
            ptrReporter2->value(code);
            break;

        case 3:
            ptrReporter3->value(~code, true);
            ptrReporter3->value(code);
            break;

        default:
            break;
    }

    // read depends on old data being cleared since it doens't always read
    // the full code size
    code = 0;
}
