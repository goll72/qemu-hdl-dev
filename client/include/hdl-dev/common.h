#ifndef HDL_DEV_COMMON_H
#define HDL_DEV_COMMON_H

#include <stdint.h>


/** General-purpose macros */

#define STR(x) STR_(x)
#define STR_(x) #x

/* Get value of an inout port with wired-or resolution */
#define GET_VALUE_INOUT_WOR(x) \
    ((x##__en & x##__out) | x)

#define GET_VALUE_INOUT_LO_Z(x) GET_VALUE_INOUT_WOR(x)

/* Get value of an inout port with wired-and resolution */
#define GET_VALUE_INOUT_WAND(x) \
    ((~x##__en | x##__out) & x)

#define GET_VALUE_INOUT_HI_Z(x) GET_VALUE_INOUT_WAND(x)

#define RISING_EDGE(prev, x) \
    (prev == 0 && x == 1)

#define FALLING_EDGE(prev, x) \
    (prev == 1 && x == 0)

#define IN(T, x) X(T, x)
#define OUT(T, x) X(T, x)
#define INOUT(T, x) \
    X(T, x);        \
    X(T, x##__en);  \
    X(T, x##__out)


/** Interface declarations for each protocol */

#ifdef __cplusplus
extern "C" {
#endif

#define X(T, x) T *x

#define PS2_PORTS         \
    INOUT(uint8_t, clk);  \
    INOUT(uint8_t, data);

struct HDLPS2Ports {
    PS2_PORTS
};

#define HDL_PS2_DEVICE_SYM hdl_ps2_dev


#define USB_PORTS         \
    /* D+ */              \
    INOUT(uint8_t, d_p);  \
    /* D- */              \
    INOUT(uint8_t, d_m);

struct HDLUSBPorts {
    USB_PORTS
};

#define HDL_USB_DEVICE_SYM hdl_usb_dev


#undef X


/** Type definitions */

typedef enum {
    STEP_TIME_INC,
    STEP_EVAL,
    STEP_EVAL_END
} hdl_step_stage_t;

#ifdef __cplusplus
}
#endif

#endif
