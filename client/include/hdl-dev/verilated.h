#ifndef HDL_DEV_VERILATED_H
#define HDL_DEV_VERILATED_H

#include <stdint.h>

#include "hdl-dev/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate memory for the top-level entity class
 * and return it as a `void *` that will be passed
 * to the other functions below as `top_`.
 */
void *hdl_dev_new(void);

/**
 * Perform initialization tasks and initialize the
 * region pointed to by `ports_` according to the
 * port definitions for the protocol in use, as
 * specified in `common.h`.
 */
void hdl_dev_init(void *top_, void *ports_);

/**
 * Return the timeprecision of the simulation as
 * the exponent in a power of 10 representation,
 * in seconds. This function can be a wrapper for
 * `top->contextp()->timeprecision()` or simply
 * tell how much time would pass after going through
 * the stages in `hdl_dev_step`, as explained in the README.
 */
int hdl_dev_timeprecision(void *top_);

/**
 * Step the simulation. This action is divided in three stages:
 *
 *   - `STEP_TIME_INC`: increment the time in the simulation context.
 *     A simple implementation would be `top->contextp()->timeInc(1)`.
 *     See the C++ files for another implementation, with an explanation
 *     for why it's used.
 *
 *  - `STEP_EVAL`: call `top->contextp()->eval_step()`.
 *
 *  - `STEP_EVAL_END`: call `top->contextp()->eval_end_step()`.
 */
void hdl_dev_step(void *top_, hdl_step_stage_t stage);

/**
 * Free the resources allocated by `hdl_dev_new`.
 */
void hdl_dev_free(void *top_);

#ifdef __cplusplus
}
#endif

#endif
