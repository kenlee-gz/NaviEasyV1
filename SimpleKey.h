#ifndef SIMPLEKEY_H_
#define SIMPLEKEY_H_

#include <stdint.h>
#include "mbed.h"

#define		KEY_PRESS			0x8000		// 按下
#define		KEY_RELEASE			0x4000		// 松开
#define		KEY_LONG			0x2000		// 长按
#define		KEY_REP				0x1000		// 重复

enum KeyStatus
{
	Key_Idle, Key_Debounce, Key_Pressed, Key_WaitRelease, Key_WaitDoubleClick,Key_IsReleased
};

class Key
{
public:
	virtual void Init() = 0;
	virtual int ScanKey()  = 0;
	virtual int WaitKeyPress() = 0;
	virtual int WaitKeyPress(uint16_t tmo) = 0;
	virtual bool GetESCorEnterKey() = 0;
	virtual ~Key(){};
};

class SimpleKey
{
private:
	DigitalIn keyin;
	KeyStatus keystatus;
	Timer timer;
	int keyBackup;
	int pollingKey();
	bool isValidKey(int key) const;

public:
	SimpleKey(PinName pin);
	int ScanKey() ;
	void Init() ;
};

#endif /* KEY_H_ */
