#include	<stdio.h>
#include	<assert.h>
#include	"mbed.h"
#include	"SimpleKey.h"


#define		KEY_DEBOUNCE		30
#define		LONGKEY_TIME		2000
#define		FIRST_REPKEY_TIME	1000
#define		REPKEY_TIME			200

SimpleKey::SimpleKey(PinName pin) : keyin(pin), keystatus(Key_Idle), keyBackup(-1)
{
	timer.start();
}

int SimpleKey::pollingKey()
{
	return (keyin.read()== 0)? 1 : -1;
}

void SimpleKey::Init()
{
	keyin.mode(PullUp);
}

bool SimpleKey::isValidKey(int key) const
{
	return key >= 0;
}

int SimpleKey::ScanKey()
{
	int key;
	int resultKey = -1;

	key = pollingKey();

	switch (keystatus)
	{
	// 按键空闲状态
	case Key_Idle:
		if (isValidKey(key))
		{
			keystatus = Key_Debounce;
			keyBackup = key;
			timer.reset();
		}
		break;

		// 按键去抖中
	case Key_Debounce:
		if (key != keyBackup)
		{
			keystatus = Key_Idle;
		}
		else if (timer.read_ms() > KEY_DEBOUNCE)
		{
			// 去抖成功，发送按键按下键值;
			resultKey = keyBackup;
			keystatus = Key_Pressed;
		}
		break;

		// 等待按键松开
	case Key_Pressed:
		if (key == -1 || key != keyBackup)
		{
			keystatus = Key_Idle;
		}
		break;

	default:
		break;
	}
	return (resultKey);
}

