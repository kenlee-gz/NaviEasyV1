#ifndef SIMPLEIN_H
#define SIMPLEIN_H

#include "platform.h"

class SimpleIn
{
public:
    SimpleIn(PinName pin)
    {
        if (pin == (PinName) NC)
        {
            mask = 0;
            return;
        }

        GPIO_TypeDef *gpio = NULL;
        switch (((uint32_t)pin >> 4) & 0xF)
        {
        case PortA:
            gpio = (GPIO_TypeDef *) GPIOA_BASE;
            break;
        case PortB:
            gpio = (GPIO_TypeDef *) GPIOB_BASE;
            break;
        case PortC:
            gpio = (GPIO_TypeDef *) GPIOC_BASE;
            break;
        case PortD:
            gpio = (GPIO_TypeDef *) GPIOD_BASE;
            break;
        default:
            error("Pinmap error: wrong port number.");
            break;
        }

        this->mask = (uint32_t) (1 << ((uint32_t) pin & 0xF));
        this->reg_in = &gpio->IDR;
    }

    int read()
    {
        if (mask == 0) return 0;
        return ((*reg_in & mask) ? 1 : 0);
    }

    operator int()
    {
        return read();
    }

protected:
    uint32_t mask;
    __IO uint32_t *reg_in;
};

#endif
