#pragma once

#include <iostream>

#include <sdbusplus/asio/connection.hpp>
#include <xyz/openbmc_project/State/Boot/Raw/server.hpp>
#include <gpiod.h>

/* The LPC snoop on port 80h is mapped to this dbus path. */
#define SNOOP_OBJECTPATH "/xyz/openbmc_project/state/boot/raw"
/* The LPC snoop on port 80h is mapped to this dbus service. */
#define SNOOP_BUSNAME "xyz.openbmc_project.State.Boot.Raw"

#define BIT(value, index) ((value >> index) & 1)

gpiod_line* led0;
gpiod_line* led1;
gpiod_line* led2;
gpiod_line* led3;
gpiod_line* led4;
gpiod_line* led5;
gpiod_line* led6;
gpiod_line* led7;

using message = sdbusplus::message::message;

static size_t totalHosts; /* Number of host */

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
    int PostCodeDisplay(uint8_t status, uint16_t host);
};

int ReadHostSelectionPos(int& pos)
{

    static boost::asio::io_service io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    auto value = conn->new_method_call(
        "xyz.openbmc_project.Misc.Ipmi", "/xyz/openbmc_project/misc/ipmi",
        "xyz.openbmc_project.Misc.Ipmi", "readHostPosition");
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

bool WritePostCode(gpiod_line* gpioLine, const char value)
{
    int ret;

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
int PostReporter ::PostCodeDisplay(uint8_t status, uint16_t host)
{
    int ret;
    char value;
    int hostSWPos = 0;

    ret = ReadHostSelectionPos(hostSWPos);
    if (ret < 0)
    {
        std::cerr << "read host position failed.\n";
        return false;
    }

    std::cerr << "HostPhyPos:" << hostSWPos << " HostIndex :" << host << "\n";
    if (hostSWPos == host)

    {
        std::cerr << "Write postcode into seven segment display\n";

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
    }

    return 0;
}

std::vector<std::unique_ptr<PostReporter>> reporters;

void PostReporter ::readPostCode(uint16_t postcode, uint16_t host)
{
    uint64_t code = 0;

    // Display postcode received from IPMB
    PostCodeDisplay(postcode, host);

    code = le64toh(postcode);
    fprintf(stderr, "Code: 0x%" PRIx64 "\n", code);

    printf("postcode = %d host = %d totalHosts = %d\n", (int)postcode,
           (int)host, (int)totalHosts);
    std::cout.flush();

    if ((host + 1) <= totalHosts)
    {
        // HACK: Always send property changed signal even for the same code
        // since we are single threaded, external users will never see the
        // first value.
        reporters[host]->value(~code, true);
        reporters[host]->value(code);
    }
    else
        fprintf(stderr, "Invalid Host :%d\n", totalHosts);

    // read depends on old data being cleared since it doens't always read
    // the full code size
    code = 0;
}
