#pragma once

#include <gpiod.hpp>
#include <iostream>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/State/Boot/Raw/server.hpp>

const static constexpr char* PropertiesIntf = "org.freedesktop.DBus.Properties";
static uint16_t numOfHost;     /* Number of host */
std::vector<gpiod_line*> leds; /*libgpiod obj for seven segment display */

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;
using PostInterface = sdbusplus::xyz::openbmc_project::State::Boot::server::Raw;
using PostObject = ServerObject<PostInterface>;

struct IpmiPostReporter : PostObject
{

    IpmiPostReporter(sdbusplus::bus::bus& bus, const char* objPath) :
        PostObject(bus, objPath), bus(bus),
        propertiesChangedSignalRaw(
            bus,
            sdbusplus::bus::match::rules::type::signal() +
                sdbusplus::bus::match::rules::member("PropertiesChanged") +
                sdbusplus::bus::match::rules::path(objPath) +
                sdbusplus::bus::match::rules::interface(PropertiesIntf),
            [this](sdbusplus::message::message& msg) {
                int hostNum = -1;
                std::string objectName;
                std::string InterfaceName;
                std::map<std::string, std::variant<uint64_t>> msgData;
                msg.read(InterfaceName, msgData);
                objectName = msg.get_path();

                size_t pos = objectName.find("raw");
                if (pos != std::string::npos)
                {
                    std::string hostNumStr = objectName.substr(pos + 3);
                    hostNum = std::stoi(hostNumStr);
                }
                // Check if it was the Value property that changed.
                auto valPropMap = msgData.find("Value");
                {
                    if (valPropMap != msgData.end())
                    {
                        int ret;
                        // write postcode into seven segment display
                        ret = postCodeDisplay(
                            std::get<uint64_t>(valPropMap->second), hostNum);
                        if (ret < 0)
                        {
                            std::cerr << "Error in display the postcode\n";
                        }
                    }
                }
            })
    {
        std::cout << "postd object created : " << objPath << std::endl;
    }

    ~IpmiPostReporter()
    {
    }

    sdbusplus::bus::bus& bus;
    sdbusplus::bus::match_t propertiesChangedSignalRaw;
    int postCodeDisplay(uint8_t status, uint16_t host);
};

// To handle multi-host postcode
std::vector<std::unique_ptr<IpmiPostReporter>> reporters;

// There should be hard or soft logic in the front panel to select the host
// to display the correposnding postcode in mult-host platform.
// This method reads the host position from the D-bus interface expose by
// other process and display it in the seven segment display based on the match.
int readHostSelectionPos(int& pos)
{

    static boost::asio::io_context io;
    auto conn = std::make_shared<sdbusplus::asio::connection>(io);

    auto method =
        conn->new_method_call("xyz.openbmc_project.Chassis.Event",
                              "/xyz/openbmc_project/Chassis/Event",
                              "org.freedesktop.DBus.Properties", "Get");

    method.append("xyz.openbmc_project.Chassis.Event", "Position");

    auto reply = conn->call(method);
    if (reply.is_method_error())
    {
        std::cerr << "Error reading host switch position" << std::endl;
        return -1;
    }

    std::variant<int> resp;
    reply.read(resp);

    pos = std::get<int>(resp);

    return 0;
}

// Configure the seven segment display connected GPIOs direction
int configGPIODirOutput(void)
{
    std::string gpio;
    // Need to define gpio names LED_POST_CODE_0 to 8 in dts file
    std::string gpioName = "LED_POST_CODE_";

    for (int iteration = 0; iteration < 8; iteration++)
    {
        gpio = gpioName + std::to_string(iteration);
        gpiod_line* led = gpiod_line_find(gpio.c_str());
        if (!led)
        {
            std::cerr << "Failed to find the gpioname : " << gpio << std::endl;
            return -1;
        }

        leds.push_back(led);
        int ret =
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

    // Write the postcode into GPIOs
    ret = gpiod_line_set_value(gpioLine, value);
    if (ret < 0)
    {
        std::cerr << "Set line output failed.\n";
        return -1;
    }

    return 0;
}

// Display the received postcode into seven segment display
int IpmiPostReporter ::postCodeDisplay(uint8_t status, uint16_t host)
{
    int ret;
    int hostSWPos = -1;

    ret = readHostSelectionPos(hostSWPos);
    if (ret < 0)
    {
        std::cerr << "Read host position failed.\n";
        return -1;
    }
    //
    if (hostSWPos == host)
    {
        /*
         * 8 GPIOs connected to seven segment display from BMC
         * to display postcode
         */
        for (int iteration = 0; iteration < 8; iteration++)
        {
            // spilt byte to write into GPIOs
            int value = (status >> iteration) & 1;
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
