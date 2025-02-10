#include <verilated.h>

#include TOP_HEADER

#include "hdl-dev/verilated.h"

extern "C" {

static uint64_t _time = 0;

void *hdl_dev_new(void)
{
    TOP *top = new TOP("");
    return static_cast<void *>(top);
}

void hdl_dev_init(void *top_, void *ports_)
{
    TOP *top = static_cast<TOP *>(top_);
    HDLPS2Ports *ports = static_cast<HDLPS2Ports *>(ports_);

#define X(T, x) ports->x = &top->x

    INOUT(_, clk);
    INOUT(_, data);

#undef X
}

int hdl_dev_timeprecision(void *top_)
{
    TOP *top = static_cast<TOP *>(top_);
    return top->contextp()->timeprecision();
}

void hdl_dev_step(void *top_, hdl_step_stage_t stage)
{
    TOP *top = static_cast<TOP *>(top_);
    VerilatedContext *ctx = top->contextp();

    switch (stage) {
        case STEP_TIME_INC:
            // This ensures multiple devices using the
            // same context will be kept in sync even
            // when `hdl_dev_step` has already been called
            // with `STEP_TIME_INC` on another device using
            // the same context. This method will work as
            // long as all instances of the same device use
            // the same context, since all such instances
            // share the `_time` variable.
            if (_time == ctx->time())
                ctx->timeInc(1);

            break;
        case STEP_EVAL:
            _time = ctx->time();
            top->eval_step();

            break;
        case STEP_EVAL_END:
            top->eval_end_step();
            break;
    }
}

void hdl_dev_free(void *top_)
{
    TOP *top = static_cast<TOP *>(top_);
    delete top;
}

int HDL_PS2_DEVICE_SYM;

}
