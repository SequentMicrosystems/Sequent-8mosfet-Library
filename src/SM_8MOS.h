/*!
 * @file SM_8MOS.h
 * 
 * Designed specifically to work with the Sequent Microsysatems 
 * HALL Current Sensor DAQ 8-Layer Stackable HAT for Raspberry Pi
 * 
 * ----> https://sequentmicrosystems.com/products/hall-current-sensor-daq-stackable-hat-for-raspberry-pi
 * 
 *   This card use CRT8 to communicate.
 *   
 *   Written by Alexandru Burcea for Sequent Microsystems
 * 
 *   Software License Agreement (BSD License)
 *
 *  Copyright (c) 2025, Sequent Microsystems 
 *  
 */
#ifndef __SM_8MOS__
#define __SM_8MOS__

#define SLAVE_OWN_ADDRESS_8MOS 0x38

#define MOS_CH_NR 8
#define PWM_SIZE_B 2
#define SM_8MOS_FREQ_MIN 16
#define SM_8MOS_FREQ_MAX 2000
#define SM_8MOS_I2C_MAX_BUFF 32



/*!
 * @brief SM_8MOS class
 */
class SM_8MOS
{
public:
	/*!
	 * @brief Class constructor.
	 */
	SM_8MOS(uint8_t stack = 0);

	/*!
	 * @brief Check card presence
	 * @return Returns true is successful
	 */
	bool begin();

	/*!
	 * @brief Return card existance status
	 * @return Returns true if card is present
	 */
	bool isAlive();
	/*!
	 * @brief Write one mosfet state .
	 * @param channel [1..8]
	 * @param val The new state true = on, false = off
	 * @return  true if succeed, false else
	 */
	bool writeChannel(uint8_t channel, uint8_t val);


    /*!
	 * @brief Write the PWM fill factor for one channel.
	 * @param channel [1..8]
	 * @param pwm [0..100] fill factor precent
	 * @return true if succeed, false else
	 */
	bool writePWM(uint8_t channel, float pwm);
	
	
	/*!
	 * @brief Write the frequency of the PWM (aply to all channels) .
	 * @param frequency in Hz [16..2000]
	 * @return true if succeed, false else
	 */
	bool writeFreqency(int frequency);
	
	/*!
	 * @brief Read one mosfet state .
	 * @param channel [1..8]
	 * @return  state true = on, false = off
	 */
	bool readChannel(uint8_t channel);


    /*!
	 * @brief Read the PWM fill factor for one channel.
	 * @param channel [1..8]
	 * @return fill factor precent
	 */
	float readPWM(uint8_t channel);
	
	
	/*!
	 * @brief Read the frequency of the PWM (aply to all channels) .
	 * @return frequency in Hz [16..2000]
	 */
	int readFreqency(int frequency);
	 
	 
private:
	uint8_t _hwAdd;
	bool _detected;
	int writeBuff(uint8_t add, uint8_t* buff, uint8_t size);
	int readBuff(uint8_t add, uint8_t* buff, uint8_t size);
	int readWord(uint8_t add, uint16_t* value);
	int readSWord(uint8_t add, int16_t* value);
	int writeByte(uint8_t add, uint8_t value);
	int writeWord(uint8_t add, uint16_t value);
	int writeSWord(uint8_t add, int16_t value);
};
#endif // __SM_16DIGIN__
