extern "C" {
#include <inttypes.h>
}

#include "Arduino.h"
#include "Wire.h"
#include "SM_8MOS.h"

#define PWM_RAW_SCALE ((float)10)

enum
{
	SM_8MOS_I2C_INPORT_REG_ADD,
	SM_8MOS_I2C_OUTPORT_REG_ADD,
	SM_8MOS_I2C_POLINV_REG_ADD,
	SM_8MOS_I2C_CFG_REG_ADD,

	SM_8MOS_I2C_MEM_DIAG_3V3_MV_ADD,
	SM_8MOS_I2C_MEM_DIAG_TEMPERATURE_ADD = SM_8MOS_I2C_MEM_DIAG_3V3_MV_ADD +2,
	SM_8MOS_I2C_MEM_PWM1,
	SM_8MOS_I2C_MODBUS_SETINGS_ADD  = SM_8MOS_I2C_MEM_PWM1 + MOS_CH_NR * PWM_SIZE_B,
	SM_8MOS_I2C_PWM_FREQ = SM_8MOS_I2C_MODBUS_SETINGS_ADD + 5,

	SM_8MOS_I2C_MEM_CPU_RESET = 0xaa,
	SM_8MOS_I2C_MEM_REVISION_HW_MAJOR_ADD ,
	SM_8MOS_I2C_MEM_REVISION_HW_MINOR_ADD,
	SM_8MOS_I2C_MEM_REVISION_MAJOR_ADD,
	SM_8MOS_I2C_MEM_REVISION_MINOR_ADD,
};

const uint8_t mosfetsRemap[8] =
{
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
	0x20,
	0x40,
	0x80};



SM_8MOS::SM_8MOS(uint8_t stack)
{
	if (stack > 7)
		stack = 7;
	_hwAdd = SLAVE_OWN_ADDRESS_8MOS + (stack ^ 0x07);
	_detected = false;
}

bool SM_8MOS::begin()
{
	uint16_t value = 0;
	Wire.begin();
	if (0 == readWord(SM_8MOS_I2C_MEM_REVISION_MAJOR_ADD, &value))
	{
		_detected = true;
	}
	return _detected;
}

bool SM_8MOS::isAlive()
{
	return _detected;
}


bool SM_8MOS::writeChannel(uint8_t channel, uint8_t val)
{
	uint8_t rawVal = 0;
	
	if (channel < 1)
	{
		channel = 1;
	}
	if (channel > MOS_CH_NR)
	{
		channel = MOS_CH_NR;
	}
	
	if(OK != readBuff(SM_8MOS_I2C_OUTPORT_REG_ADD, &rawVal, 1))
	{
		return false;
	}
	if (val == 0)//reverse polarity
	{
		rawVal |= mosfetsRemap[channel - 1];
	}
	else
	{
		rawVal &= ~mosfetsRemap[channel - 1];
	}
	
	if(OK != writeBuff(SM_8MOS_I2C_OUTPORT_REG_ADD, &rawVal, 1))
	{
		return false;
	}
	
	return true;
}


bool SM_8MOS::writePWM(uint8_t channel, float pwm)
{
	int16_t rawVal = 0;
	
	if (channel < 1)
	{
		channel = 1;
	}
	if (channel > MOS_CH_NR)
	{
		channel = MOS_CH_NR;
	}
	if(pwm < 0)
	{
		pwm = 0;
	}
	if(pwm > 100)
	{
		pwm = 100;
	}
	rawVal = (uint16_t)(pwm * PWM_RAW_SCALE);
	
	if(OK != writeWord(SM_8MOS_I2C_MEM_PWM1 + PWM_SIZE_B * (channel - 1), rawVal))
	{
		return false;
	}
	
	return true;
}

bool SM_8MOS::writeFreqency(int frequency)
{
	if(frequency < SM_8MOS_FREQ_MIN)
	{
		frequency = SM_8MOS_FREQ_MIN;
	}
	if(frequency > SM_8MOS_FREQ_MAX)
	{
		frequency = SM_8MOS_FREQ_MAX;
	}
	if(OK != writeWord(SM_8MOS_I2C_PWM_FREQ, (uint16_t)frequency))
	{
		return false;
	}
	
	return true;
}

bool SM_8MOS::readChannel(uint8_t channel)
{
	uint8_t rawVal = 0;
	
	if (channel < 1)
	{
		channel = 1;
	}
	if (channel > MOS_CH_NR)
	{
		channel = MOS_CH_NR;
	}
	
	if(OK != readBuff(SM_8MOS_I2C_OUTPORT_REG_ADD, &rawVal, 1))
	{
		return false;
	}
	if(rawVal & mosfetsRemap[channel - 1])
	{
		return false;
	}
	return true;
}

float SM_8MOS::readPWM(uint8_t channel)
{
	uint16_t rawVal = 0;
	
	if (channel < 1)
	{
		channel = 1;
	}
	if (channel > MOS_CH_NR)
	{
		channel = MOS_CH_NR;
	}
	
	if(OK != readWord(SM_8MOS_I2C_MEM_PWM1 + PWM_SIZE_B * (channel - 1), &rawVal))
	{
		return -1;
	}
	
	return((float)rawVal/PWM_RAW_SCALE);
	
}


int SM_8MOS::readFreqency(int frequency)
{
	uint16_t rawVal = 0;
	
	if(OK != readWord(SM_8MOS_I2C_PWM_FREQ, &rawVal))
	{
		return -1;
	}
	
	return (int)rawVal;
}



/*
 ***************** 8mosfet_I2C access functions ****************************
 **********************************************************************
 */
 int SM_8MOS::writeBuff(uint8_t add, uint8_t* buff, uint8_t size)
 {
	int i = 0;

	if(size > SM_8MOS_I2C_MAX_BUFF)
	{
		return -1;
	}
	
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	for (i = 0; i < size; i++)
	{
		Wire.write(buff[i]);
	}
	return Wire.endTransmission();
 }
 
int SM_8MOS::readBuff(uint8_t add, uint8_t* buff, uint8_t size)
{
	uint8_t i = 0;

	if (0 == buff)
	{
		return -1;
	}
	if(size > SM_8MOS_I2C_MAX_BUFF)
	{
		return -1;
	}
	
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	if (Wire.endTransmission() != 0)
	{
		return -1;
	}
	Wire.requestFrom(_hwAdd, size);
	if (size <= Wire.available())
	{
		for(i = 0; i < size; i++)
		{
			buff[i] = Wire.read();
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

int SM_8MOS::writeByte(uint8_t add, uint8_t value)
{
  Wire.begin();
  Wire.beginTransmission(_hwAdd);
  Wire.write(add);
  Wire.write(value);
  return Wire.endTransmission();
}
	
int SM_8MOS::readWord(uint8_t add, uint16_t* value)
{
	uint8_t buff[2];

	if (0 == value)
	{
		return -1;
	}
	//Wire.begin();
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	if (Wire.endTransmission() != 0)
	{
		return -1;
	}
	Wire.requestFrom(_hwAdd, (uint8_t)2);
	if (2 <= Wire.available())
	{
		buff[0] = Wire.read();
		buff[1] = Wire.read();
	}
	else
	{
		return -1;
	}
	memcpy(value, buff, 2);
	return 0;
}

int SM_8MOS::readSWord(uint8_t add, int16_t* value)
{
	uint8_t buff[2];

	if (0 == value)
	{
		return -1;
	}
	//Wire.begin();
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	if (Wire.endTransmission() != 0)
	{
		return -1;
	}
	Wire.requestFrom(_hwAdd, (uint8_t)2);
	if (2 <= Wire.available())
	{
		buff[0] = Wire.read();
		buff[1] = Wire.read();
	}
	else
	{
		return -1;
	}
	memcpy(value, buff, 2);
	return 0;
}

int SM_8MOS::writeWord(uint8_t add, uint16_t value)
{
	uint8_t buff[2];

	memcpy(buff, &value, 2);
	//Wire.begin();
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	Wire.write(buff[0]);
	Wire.write(buff[1]);
	return Wire.endTransmission();

}

int SM_8MOS::writeSWord(uint8_t add, int16_t value)
{
	uint8_t buff[2];

	memcpy(buff, &value, 2);
	//Wire.begin();
	Wire.beginTransmission(_hwAdd);
	Wire.write(add);
	Wire.write(buff[0]);
	Wire.write(buff[1]);
	return Wire.endTransmission();

}

 
 

