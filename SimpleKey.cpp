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
	// ��������״̬
	case Key_Idle:
		if (isValidKey(key))
		{
			keystatus = Key_Debounce;
			keyBackup = key;
			timer.reset();
		}
		break;

		// ����ȥ����
	case Key_Debounce:
		if (key != keyBackup)
		{
			keystatus = Key_Idle;
		}
		else if (timer.read_ms() > KEY_DEBOUNCE)
		{
			// ȥ���ɹ������Ͱ������¼�ֵ;
			resultKey = keyBackup;
			keystatus = Key_Pressed;
		}
		break;

		// �ȴ������ɿ�
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

