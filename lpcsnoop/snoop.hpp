#pragma once

#include <unistd.h>

#include <iostream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/State/Boot/Raw/server.hpp>

/* The LPC snoop on port 80h is mapped to this dbus path. */
#define SNOOP_OBJECTPATH "/xyz/openbmc_project/state/boot/raw"
/* The LPC snoop on port 80h is mapped to this dbus service. */
#define SNOOP_BUSNAME "xyz.openbmc_project.State.Boot.Raw"

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

    void readPostcode(uint16_t postcode, uint16_t host) override;
    int write_device(const char* device, const char* value);
    int post_display(uint8_t status);
};

// class PostReporter;
PostReporter* ptrReporter;
PostReporter* ptrReporter1;
PostReporter* ptrReporter2;
PostReporter* ptrReporter3;
#if 1
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
#endif

#define BIT(value, index) ((value >> index) & 1)

int PostReporter ::write_device(const char* device, const char* value)
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
int PostReporter ::post_display(uint8_t status)
{
#if 0
   
   if (BIT(status, 0))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_0", val);

     if (BIT(status, 1))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_1", val);

   if (BIT(status, 2))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_2", val);
   if (BIT(status, 3))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_3", val);
   if (BIT(status, 4))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_4", val);
   if (BIT(status, 5))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_5", val);
   if (BIT(status, 6))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_6", val);

   if (BIT(status, 7))
    {
        val = "1";
    }
    else
    {
        val = "0";
    }
  setGPIOOutput("LED_POST_CODE_7", val);
#endif

#if 1
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
#endif
}
void PostReporter ::readPostcode(uint16_t postcode, uint16_t host)
{
    uint64_t code = 0;

    code = le64toh(postcode);

    fprintf(stderr, "Code: 0x%" PRIx64 "\n", code);

    printf("postcode= %d host=%d\n", (int)postcode, (int)host);

    printf("HCL4\n");

    std::cout.flush();

#if 0

#if 1
    switch (host)
    {
        case 0:
            // HACK: Always send property changed signal even for the same code
            // since we are single threaded, external users will never see the
            // first value.
            ptrReporter->value(~code, true);
            ptrReporter->value(code);
            break;

        case 2:
            ptrReporter1->value(~code, true);
            ptrReporter1->value(code);
            break;

        case 3:
            ptrReporter2->value(~code, true);
            ptrReporter2->value(code);
            break;

        case 4:
            ptrReporter3->value(~code, true);
            ptrReporter3->value(code);
            break;

        default:
            break;
    }
#endif

#endif
    // read depends on old data being cleared since it doens't always read
    // the full code size
    code = 0;

    // Display postcode received from IPMB
    // post_display(postcode);
}
