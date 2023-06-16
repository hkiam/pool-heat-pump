#pragma once
#include <Arduino.h>

// Define constants
#define REPEAT_SEND 3
#define REPEAT_DELAY_US 243000
#define RECVPIN D7
#define SENDPIN D7
#define leading_pulse_length 9300
#define leading_space_length 4700
#define pulse_length 1152
#define space_one_length 1141
#define space_zero__length 3180
#define maxnoise 500
#define framesize 10
#define rxbuffersize 5

// Define a function-like macro to check if a value is within a certain range of an expected value
#define CHECK(value, expected) abs(long(value-expected))< maxnoise 

enum capture_state {wait_leading_pulse, wait_leading_space, wait_pulse, wait_bit};

typedef unsigned char byte;

// Convert a byte array to a hex string
String byteArrayToHexString(const unsigned char *data, int len);

// Convert a hex string to a byte array
int hexStringToByteArray(const char* hexString, unsigned int stringLength,  byte* byteArray, size_t byteArraySize);

// Calculate the checksum of a byte array
int checksum(byte * buffer, int len);

class poolheater
{
private:
    byte frames[rxbuffersize][framesize]; // Buffer to store received frames
    int rxpos; // Position in the buffer where the next received frame should be stored
    int txpos; // Position in the buffer where the next frame to be sent is stored

    unsigned short bitbuffer; // Buffer to store received bits
    unsigned int bytepos; // Position in the buffer where the next byte to be assembled is stored
    capture_state state; // State machine variable to keep track of the receiving process

    // Append a byte to the buffer
    void appendByte(byte d);

    // Append a bit to the buffer
    void appendBit(bool bit, unsigned long duration);

    // Reset the receiving state machine
    void reset();

    // Handle an error in the receiving process
    void handleerror(bool bit, unsigned long duration);

public:
    poolheater(/* args */);
    ~poolheater();

    // Enable or disable receiving
    void enableRx(bool on);

    // Send a frame
    int sendFrame(byte * frame, unsigned timeout);

    // Receive a frame
    bool receiveFrame(byte * frame);

    // Declare a friend function that will handle interrupt service routine for receiving bits
    friend void rxBitISR(void * arg);

    // Last error message
    String lasterror = "";

    // Time of the last received frame
    unsigned long lastframerecvtime;
};
