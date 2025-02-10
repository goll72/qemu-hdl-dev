#ifndef HDL_DEV_COMMON_H
#define HDL_DEV_COMMON_H

#include <stdint.h>


/** General-purpose macros */

#define STR(x) STR_(x)
#define STR_(x) #x

#define GET_VALUE_INOUT_HI_Z(x) \
    x##_dir == 0                \
        ? x                     \
        : x##__en == 1          \
            ? x##__out          \
            : 1

#define FALLING_EDGE(prev, x) \
    (prev == 1 && x == 0)

#define IN(T, x) X(T, x)
#define OUT(T, x) X(T, x)
#define INOUT(T, x) \
    X(T, x);        \
    X(T, x##_dir);  \
    X(T, x##__en);  \
    X(T, x##__out)


/** Interface declarations for each protocol */

#ifdef __cplusplus
extern "C" {
#endif

#define X(T, x) T *x


struct HDLPS2Ports {
    INOUT(uint8_t, clk);
    INOUT(uint8_t, data);
};

#define HDL_PS2_DEVICE_SYM hdl_ps2_dev


struct HDLUSBPorts {
    // D+
    INOUT(uint8_t, d_p);
    // D-
    INOUT(uint8_t, d_m);
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
