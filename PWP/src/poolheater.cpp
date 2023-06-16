#include "poolheater.h"
/**
 * Converts a byte array to a hexadecimal string.
 * 
 * @param data A pointer to the byte array.
 * @param len The length of the byte array.
 * @return A hexadecimal string representation of the byte array.
 */
String byteArrayToHexString(const unsigned char *data, int len)
{
    String result = "";

    // Iterate through each byte in the array.
    for (int i(0); i < len; ++i) {
        // If the byte is less than 0x10, pad it with a leading 0.
        if (data[i] < 0x10) {
            result += '0';
        }

        // Convert the byte to a hexadecimal string and append it to the result.
        result += String(data[i], HEX);
    }

    return result;
}


/**
 * Convert a hexadecimal string to a byte array.
 * @param hexString The hexadecimal string to convert.
 * @param stringLength The length of the hexadecimal string.
 * @param byteArray The byte array to store the converted bytes in.
 * @param byteArraySize The size of the byte array.
 * @return The number of bytes converted.
 */
int hexStringToByteArray(const char* hexString, unsigned int stringLength,  byte* byteArray, size_t byteArraySize) {
  // Prüfen, ob der Hex-String eine gerade Anzahl von Zeichen hat
  if (stringLength % 2 != 0) {
    Serial.print("invalid hexstring. (");
    Serial.print(stringLength);
    Serial.println(")");

    return 0;
  }

  // Prüfen, ob das Byte-Array groß genug ist
  if (byteArraySize < stringLength / 2) {
    Serial.println("buffer to small.");
    return 0;
  }

  // Convert each pair of hexadecimal characters to a byte and store in the byte array
  for (size_t i = 0; i < stringLength; i += 2) {
    char hexByte[3];
    strncpy(hexByte, &hexString[i], 2);
    hexByte[2] = '\0';
    byteArray[i / 2] = strtol(hexByte, NULL, 16);
  }

  // Return the number of bytes converted
  return stringLength / 2;
}



/**
 * Calculates the checksum of a byte buffer.
 * 
 * @param buffer: The buffer to calculate the checksum for.
 * @param len: The length of the buffer.
 * 
 * @return The checksum of the buffer.
 */
int checksum(byte * buffer, int len){
    byte res = 0;
    for(int i=0;i<len;i++) res+=buffer[i];
    return res;
}


/**
 * @brief Reverses the bits of a byte
 * 
 * This function takes a byte and reverses the order of its bits.
 * 
 * @param d The byte to reverse
 * @return The reversed byte
 */
byte IRAM_ATTR reverse_bit(byte d){
    byte result = 0;
    // Iterate through each bit in the byte
    for(int i=0;i<8;i++){
        // Shift the result left by one bit and OR it with the least significant bit of the input byte
        result = (result << 1) | (d & 1);
        // Shift the input byte right by one bit to move on to the next bit
        d >>= 1;
    } 
    // Return the reversed byte
    return result;
}


/**
 * @brief Adds a byte to the frames buffer
 * 
 * @param d Byte to be added
 */
void IRAM_ATTR poolheater::appendByte(byte d){

    // Add the byte to the current frame at the current byte position
    frames[rxpos][bytepos++] = d;

    // If the byte position is at the end of the frame
    if(bytepos == framesize){

        // Record the time the last frame was received
        lastframerecvtime = micros();

        // Move to the next buffer position
        rxpos=(rxpos+1)%rxbuffersize;

        // If the buffer has overflowed
        if(rxpos==txpos){
            //error Buffer overflow
        }

        // Reset the byte position
        bytepos = 0;
    }
}


/**
 * Receives a frame from the buffer and checks its checksum.
 * @param frame: the buffer where the received frame will be stored
 * @returns true if the checksum is correct, false otherwise
 */
bool poolheater::receiveFrame(byte * frame) {
    // Check if the transmit and receive positions are the same, indicating no data
    if (txpos == rxpos) {
        //Serial.println("no data");
        return false;
    }

    // Copy the frame from the buffer to the given frame buffer
    memcpy(frame, frames[txpos], framesize);

    // Increment the transmit position and wrap around if necessary
    txpos = (txpos + 1) % rxbuffersize;

    // Invert the data if the first byte is less than 0x40
    if (frame[0] < 0x40) {
        Serial.println("invert data");
        for (int i = 0; i < 10; i++) {
            frame[i] ^= 0xFF;
        }
    }

    // Check if the checksum is correct
    bool chksum_ok = checksum(frame+1, 8) == frame[9];
    if (!chksum_ok) {
        Serial.println("wrong checksum");
    }
    return chksum_ok;
}


unsigned long last_isr_time = 0;
/**
 * @brief Interrupt Service Routine (ISR) for receiving bits over a communication channel
 * 
 * @param arg Pointer to the object instance of the poolheater class
 */
void IRAM_ATTR rxBitISR(void * arg){
    // Cast the void pointer to the object instance of the class
    poolheater* self = reinterpret_cast<poolheater*>(arg);

    // Calculate the duration since the last ISR was triggered
    unsigned long duration = micros() - last_isr_time;

    // Read the new state of the receive pin
    int newpinstate = digitalRead(RECVPIN);

    // Invert the new state of the receive pin and append it to the buffer along with its duration
    self->appendBit(!newpinstate,duration);

    // Update the timestamp of the last ISR trigger
    last_isr_time = micros(); 
}


/**
 * Enable or disable reception of data on the RX pin.
 *
 * @param on If true, enable reception. If false, disable reception.
 */
void poolheater::enableRx(bool on){
    if (on) {
        pinMode(RECVPIN, INPUT);  // Set the RX pin as input.
        attachInterruptArg(RECVPIN, rxBitISR, this, CHANGE);  // Attach the interrupt handler.
    } else {
        detachInterrupt(RECVPIN);  // Detach the interrupt handler.
        pinMode(SENDPIN, OUTPUT);  // Set the TX pin as output. The active pin state will then result in a high-impedance state.
    }
    reset();  // Reset internal state.
}


/**
 * Sends a frame to the pool heater.
 * @param frame A byte array containing the frame to send.
 * @param timeout The maximum amount of time to wait for a response.
 * @return 1 if the frame was sent successfully, 0 otherwise.
 */
int poolheater::sendFrame(byte * frame, unsigned int timeout){

    // Loop through the timeout period, checking for a response
    for (unsigned i = 0; i < timeout; i++){

        // Calculate the time since the last frame was received
        int lastrcv = micros()-lastframerecvtime; 

        // If the time since the last frame is within an acceptable range...
        if( lastrcv < 1000000 && lastrcv > 15000){ // TODO, check time     

            // Disable receive mode before sending
            enableRx(false);

            // Set the send pin high
            digitalWrite(SENDPIN, HIGH);

            // Add the checksum to the frame
            frame[9] = checksum(frame+1,8);

            // Invert the data for transmission
            for(int i=0;i<10;i++) frame[i]^=0xFF;

            // Send the frame multiple times for redundancy
            for (size_t i = 0; i < REPEAT_SEND; i++){                            

                // Send the leading pulse
                digitalWrite(SENDPIN, LOW);
                delayMicroseconds(leading_pulse_length);

                // Send the leading space
                digitalWrite(SENDPIN, HIGH);
                delayMicroseconds(leading_space_length);

                // Send each byte of the frame, one bit at a time
                for (size_t j = 0; j < framesize; j++){                    
                    for (size_t k = 0; k < 8; k++){
                        digitalWrite(SENDPIN, LOW);
                        delayMicroseconds(pulse_length);

                        // Determine the value of the current bit
                        bool bitvalue = (frame[j]>>k)&1;

                        // Send the appropriate space length based on the bit value
                        delayMicroseconds(bitvalue?space_one_length:space_zero__length);
                        digitalWrite(SENDPIN, HIGH);
                    }                                    
                }

                // Send the ending pulse
                digitalWrite(SENDPIN, LOW);         
                delayMicroseconds(pulse_length);
                digitalWrite(SENDPIN, HIGH);

                // Delay before sending the next repetition
                delayMicroseconds(REPEAT_DELAY_US);
            }

            // Re-enable receive mode before returning
            enableRx(true);
            return 1;
        }

        // If no response was received, wait one millisecond before trying again
        delay(1);        
    }

    // If the timeout period has elapsed, return 0 to indicate failure
    return 0;
}


/**
 * @brief Handles error state and saves the last error
 * 
 * @param bit The bit of error
 * @param duration The duration of the error
 */
void IRAM_ATTR poolheater::handleerror(bool bit, unsigned long duration){
    char tmp[100];
    // Formats the error message
    sprintf(tmp,"error state:%d bit:%d duration:%lu\n", state,bit,duration);
    // Saves the error message
    lasterror = String(tmp);
    // Resets the state
    reset();
}


/**
 * @brief Appends a bit to the internal buffer, using the given pulse duration and current state
 * 
 * @param bit the bit to append
 * @param duration the duration of the pulse that represents the bit
 */
void IRAM_ATTR poolheater::appendBit(bool bit, unsigned long duration){
    // if the duration is too long, reset the decoding state
    if(duration > leading_space_length+maxnoise){
        reset();
    }

    switch(state){
        case wait_leading_pulse:
            // if the pulse represents a logical 0 and has the expected length, move to the next state
            if(CHECK(duration,leading_pulse_length) && !bit){
                state = wait_leading_space;
                return;
            }else{
                // otherwise do nothing and wait for the next pulse
                return;
            }

        case wait_leading_space:
            // if the pulse represents a logical 1 and has the expected length, move to the next state
            if(CHECK(duration,leading_space_length) && bit){
                state = wait_pulse;
                return;

            }else{
                // otherwise handle the error and reset the decoding state
                handleerror(bit,duration);
                return;
            }
        case wait_pulse:
            // if the pulse represents a logical 0 and has the expected length, move to the next state
            if(CHECK(duration,pulse_length) && !bit){
                state = wait_bit;
                return;
            }else{
                // otherwise handle the error and reset the decoding state
                handleerror(bit,duration);
                return;
            }
        case wait_bit:{
            // if the bit is not valid, handle the error and reset the decoding state
            if(!bit ){
                handleerror(bit,duration);
                return;
            }

            state = wait_pulse;
            byte bit = 0;
            // if the space duration matches the expected duration for a logical 1, set the bit to 1
            if (CHECK(duration,space_one_length)) {
                bit = 1;
            }
            // append the bit to the internal buffer
            bitbuffer = (bitbuffer<<1) | bit;
            // if the buffer is full, append the corresponding byte to the output buffer
            if (bitbuffer & 0b100000000){
                appendByte(reverse_bit(bitbuffer&0xFF));
                bitbuffer = 1;
            }
        }
    }
}


/**
 * @brief Reset the pool heater state machine
 * 
 */
void IRAM_ATTR poolheater::reset(){
    // Reset the byte position
    bytepos = 0;
    // Reset the bit buffer
    bitbuffer = 1;
    // Set the state to wait for the leading pulse
    state = wait_leading_pulse;
}


poolheater::poolheater(/* args */)
{
    rxpos = txpos = 0;
    lastframerecvtime = 0;
    reset();
}

poolheater::~poolheater()
{
}