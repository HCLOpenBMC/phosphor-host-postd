/**
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lpcsnoop/snoop.hpp"

#include <endian.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/epoll.h>
#include <systemd/sd-event.h>
#include <unistd.h>

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/event.hpp>
#include <sdeventplus/source/io.hpp>
#include <thread>

static const char* snoopFilename = "/dev/aspeed-lpc-snoop0";
static size_t codeSize = 1; /* Size of each POST code in bytes */

static void usage(const char* name)
{
    fprintf(stderr,
            "Usage: %s [-d <DEVICE>]\n"
            "  -b, --bytes <SIZE>     set POST code length to <SIZE> bytes. "
            "Default is %zu\n"
            "  -d, --device <DEVICE>  use <DEVICE> file. Default is '%s'\n"
            "  -v, --verbose  Prints verbose information while running\n\n",
            name, codeSize, snoopFilename);
}

/*
 * Callback handling IO event from the POST code fd. i.e. there is new
 * POST code available to read.
 */
void PostCodeEventHandler(sdeventplus::source::IO& s, int postFd, uint32_t,
                          PostReporter* reporter, bool verbose)
{
    uint64_t code = 0;
    ssize_t readb;
    while ((readb = read(postFd, &code, codeSize)) > 0)
    {
        code = le64toh(code);
        if (verbose)
        {
            fprintf(stderr, "Code: 0x%" PRIx64 "\n", code);
        }
        // HACK: Always send property changed signal even for the same code
        // since we are single threaded, external users will never see the
        // first value.
        reporter->value(~code, true);
        reporter->value(code);

        // read depends on old data being cleared since it doens't always read
        // the full code size
        code = 0;
    }

    if (readb < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        return;
    }

    /* Read failure. */
    if (readb == 0)
    {
        fprintf(stderr, "Unexpected EOF reading postcode\n");
    }
    else
    {
        fprintf(stderr, "Failed to read postcode: %s\n", strerror(errno));
    }
    s.get_event().exit(1);
}

#define GPIO_VAL "/sys/class/gpio/gpio%d/value"
#define GPIO_BASE_NUM 280

#define GPIO_POSTCODE_0 (48 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_1 (49 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_2 (50 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_3 (51 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_4 (124 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_5 (125 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_6 (126 + GPIO_BASE_NUM)
#define GPIO_POSTCODE_7 (127 + GPIO_BASE_NUM)

#define BIT(value, index) ((value >> index) & 1)

static int write_device(const char* device, const char* value)
{
    FILE* fp;
    int rc;

    std::cout << "write_device()" << std::endl;

    fp = fopen(device, "w");
    if (!fp)
    {
        int err = errno;

        std::cout << "failed to open device for write :" << device << std::endl;

        return err;
    }

    rc = fputs(value, fp);
    fclose(fp);

    if (rc < 0)
    {

        std::cout << "failed to write device" << device << std::endl;

        return ENOENT;
    }
    else
    {
        return 0;
    }
}

// Display the given POST code using GPIO port
static int post_display(uint8_t status)
{
    char path[64] = {0};
    int ret;
    const char* val;

    std::cout << "post_display: status is" << status << std::endl;

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_0);

    if (BIT(status, 0))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_1);
    if (BIT(status, 1))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_2);
    if (BIT(status, 2))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_3);
    if (BIT(status, 3))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_4);
    if (BIT(status, 4))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_5);
    if (BIT(status, 5))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_6);
    if (BIT(status, 6))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

    sprintf(path, GPIO_VAL, GPIO_POSTCODE_7);
    if (BIT(status, 7))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }

    ret = write_device(path, val);
    if (ret)
    {
        goto post_exit;
    }

post_exit:
    if (ret)
    {
        std::cout << "write_device failed for" << path << std::endl;
        return -1;
    }
    else
    {
        return 0;
    }
}

PostReporter* ptrReporter;

static void readPostcode(uint8_t postcode, uint8_t host)
{
    uint64_t code = 0;

    code = le64toh(postcode);

    fprintf(stderr, "Code: 0x%" PRIx64 "\n", code);

    switch (host)
    {
        case 1:
            // HACK: Always send property changed signal even for the same code
            // since we are single threaded, external users will never see the
            // first value.
            ptrReporter->value0(~code, true);
            ptrReporter->value0(code);
            break;

        case 2:
            ptrReporter->value1(~code, true);
            ptrReporter->value1(code);
            break;

        case 3:
            ptrReporter->value0(~code, true);
            ptrReporter->value0(code);
            break;

        case 4:
            ptrReporter->value1(~code, true);
            ptrReporter->value1(code);
            break;

        default:
            break;
    }

    // read depends on old data being cleared since it doens't always read
    // the full code size
    code = 0;

    // Display postcode received from IPMB
    post_display(postcode);
}

void PostCodeIpmiHandler(const char* snoopObject, const char* snoopDbus,
                         bool deferSignals)
{
    int rc = 0;

    auto bus = sdbusplus::bus::new_default();

    // Add systemd object manager.
    sdbusplus::server::manager::manager(bus, snoopObject);

    PostReporter reporter(bus, snoopObject, deferSignals);
    reporter.emit_object_added();
    bus.request_name(snoopDbus);

    ptrReporter = &reporter;

    try
    {
        sdeventplus::Event event = sdeventplus::Event::get_default();
        // Enable bus to handle incoming IO and bus events
        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);
        rc = event.loop();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
    }
}

/*
 * TODO(venture): this only listens one of the possible snoop ports, but
 * doesn't share the namespace.
 *
 * This polls() the lpc snoop character device and it owns the dbus object
 * whose value is the latest port 80h value.
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    int opt;
    int postFd = -1;

    /*
     * These string constants are only used in this method within this object
     * and this object is the only object feeding into the final binary.
     *
     * If however, another object is added to this binary it would be proper
     * to move these declarations to be global and extern to the other object.
     */
    const char* snoopObject = SNOOP_OBJECTPATH;
    const char* snoopDbus = SNOOP_BUSNAME;

    bool deferSignals = true;
    bool verbose = false;

    // clang-format off
    static const struct option long_options[] = {
        {"bytes",  required_argument, NULL, 'b'},
        {"device", required_argument, NULL, 'd'},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };
    // clang-format on

    while ((opt = getopt_long(argc, argv, "b:d:v", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 0:
                break;
            case 'b':
                codeSize = atoi(optarg);

                if (codeSize < 1 || codeSize > 8)
                {
                    fprintf(stderr,
                            "Invalid POST code size '%s'. Must be "
                            "an integer from 1 to 8.\n",
                            optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                snoopFilename = optarg;
                PostCodeIpmiHandler(snoopObject, snoopDbus, deferSignals);
                break;
            case 'v':
                verbose = true;
                break;
            case 'i':
                verbose = true;
                PostCodeIpmiHandler(snoopObject, snoopDbus, deferSignals);
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    postFd = open(snoopFilename, O_NONBLOCK);
    if (postFd < 0)
    {
        fprintf(stderr, "Unable to open: %s\n", snoopFilename);
        return -1;
    }

    auto bus = sdbusplus::bus::new_default();

    // Add systemd object manager.
    sdbusplus::server::manager::manager(bus, snoopObject);

    PostReporter reporter(bus, snoopObject, deferSignals);
    reporter.emit_object_added();
    bus.request_name(snoopDbus);

    // Create sdevent and add IO source
    try
    {
        sdeventplus::Event event = sdeventplus::Event::get_default();
        sdeventplus::source::IO reporterSource(
            event, postFd, EPOLLIN | EPOLLET,
            std::bind(PostCodeEventHandler, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3, &reporter,
                      verbose));
        // Enable bus to handle incoming IO and bus events
        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);
        rc = event.loop();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
    }

    if (postFd > -1)
    {
        close(postFd);
    }

    return rc;
}
