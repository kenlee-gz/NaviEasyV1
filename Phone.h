/******************************************************************************
 * @file     	Phone.h
 * @brief    
 * @version  	1.0.0
 * @date     	2016Äê6ÔÂ22ÈÕ
 *
 * @note
 * Copyright (C) 2016 YUWEI Corp. All rights reserved.
 ******************************************************************************/

#ifndef PHONE_H_
#define PHONE_H_

class Phone
{
public:
    virtual ~Phone(){};
    virtual bool IsConnect() = 0;
};

#endif /* PHONE_H_ */
