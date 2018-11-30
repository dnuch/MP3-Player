//
// Created by drnuc on 9/8/2018.
//

#ifndef GPIOINTERRUPTS_H
#define GPIOINTERRUPTS_H

#include "lpc_isr.h"
#include "stdint.h"
#include "LPC17xx.h"
#include "GPIODriver.h"
#include "singleton.h"
#include "tasks.hpp"

enum InterruptCondition
{
    kRisingEdge,
    kFallingEdge,
    kBothEdges,
};

/**
 * Typdef a function pointer which will help in code readability
 * For example, with a function foo(), you can do this:
 * IsrPointer function_ptr = foo;
 * OR
 * IsrPointer function_ptr = &foo;
 */
typedef void (*IsrPointer)(void);

class GPIOInterrupts : public Singleton<GPIOInterrupts>
{
public:
    /**
    * Optional: LabGpioInterrupts could be a singleton class, meaning, only one instance can exist at a time.
    * Look up how to implement this. It is best to not allocate memory in the constructor and leave complex
    * code to the Initialize() that you call in your main()
    */
    GPIOInterrupts() = default;

    /**
     * This should configure NVIC to notice EINT3 IRQs; use NVIC_EnableIRQ()
     */
    void Initialize();

    /**
     * This handler should place a function pointer within the lookup table for the HandleInterrupt() to find.
     *
     * @param[in] port         specify the GPIO port, and 1st dimension of the lookup matrix
     * @param[in] pin          specify the GPIO pin to assign an ISR to, and 2nd dimension of the lookup matrix
     * @param[in] pin_isr      function to run when the interrupt event occurs
     * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
     * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was successful
     */
    bool AttachInterruptHandler(uint8_t port, uint8_t pin, IsrPointer pin_isr, InterruptCondition condition);

    /**
     * This function is invoked by the CPU (through Eint3Handler) asynchronously when a Port/Pin
     * interrupt occurs. This function is where you will check the Port status, such as IO0IntStatF,
     * and then invoke the user's registered callback and find the entry in your lookup table.
     *
     * VERY IMPORTANT!
     *  - Be sure to clear the interrupt flag that caused this interrupt, or this function will be called
     *    repetitively and lock your system.
     *  - NOTE that your code needs to be able to handle two GPIO interrupts occurring at the same time.
     */
    void HandleInterrupt();

private:
    /**
     * Allocate a lookup table matrix here of function pointers (avoid dynamic allocation)
     * Upon AttachInterruptHandler(), you will store the user's function callback
     * Upon the EINT3 interrupt, you will find out which callback to invoke based on Port/Pin status.
     * port 0 = index 0, port 2 = index 1
     */
    IsrPointer pin_isr_map[2][32];
    friend class Singleton<GPIOInterrupts>;  /// Friend class used for Singleton Template
};

/* Since we have a C++ class handle an interrupt, we need to setup a C function delegate to invoke it
 * So here is the skeleton code that you can reference.
 * This function will simply delegate the interrupt handling to our C++ class
 * The CPU interrupt should be attached to this function through isr_register()
 */

/**
 * main() should register C function as callback for the EINT3
 * This is because we cannot register a C++ function as a callback through isr_register()
 *
 * There are workarounds, such as static functions inside a class, but that design is
 * not covered in this assignment.
 */

// global semaphore for LED tasks; port 0 = index 0, port 2 = index 1
extern SemaphoreHandle_t xSemaphore[2][32];

// global object
extern GPIOInterrupts & gpioInterrupts;
// delegate c interrupt to call c++ function
extern void eint3Handler(void);

#endif //GPIOINTERRUPTS_H
