#include <stdio.h>     
#include <stdlib.h> 
#include <iomanip>
#include <sstream>
#include <unordered_set>

std::unordered_set<std::string> stringHashSet;


std::string byteArrayToHexString(const unsigned char *data, int len)
{
     std::stringstream ss;
     ss << std::hex;
     for( int i(0) ; i < len; ++i ){
        ss << std::setw(2) << std::setfill('0') << (int)data[i] << " "; 
     }
     return ss.str();
}



typedef unsigned char byte;


#define leading_pulse_length 9000
#define leading_space_length 4500
#define pulse_length 1150
#define space_one_length 1150
#define space_zero__length 3200
#define maxnoise 500
#define maxframesize 10

#define CHECK(value, expected) abs(long(value-expected))< maxnoise 

enum capture_state {wait_leading_pulse, wait_leading_space, wait_pulse, wait_bit};
       
byte reverse_bit(byte d){
    byte result = 0;
    for(int i=0;i<8;i++){
        result = (result << 1) | (d & 1);
        d >>= 1;
    } 
    return result;
}

class pulsedecoder{
    private:
        byte frame[maxframesize];
        unsigned short bitbuffer;
        unsigned int bytepos;
        capture_state state;

        int add(byte d){
            frame[bytepos++] = d;
            return bytepos;
        }

        void reporterror(bool bit, unsigned long duration){
            printf("error state:%d bit:%d duration:%lu\n", state,bit,duration);
            reset();
            //exit(1);
        }

    public:
        pulsedecoder(){
            reset();
        }

        void reset(){
            bytepos = 0;
            bitbuffer = 1;
            state = wait_leading_pulse;
        }

        byte * getframe(){
            return frame;
        }



        int add(bool bit, unsigned long duration){
            if(bytepos==maxframesize || duration > leading_space_length+maxnoise){
                reset();
            }
            
            switch(state){
                case wait_leading_pulse:
                    if(CHECK(duration,leading_pulse_length) && !bit){
                        state = wait_leading_space;
                        return 0;
                    }else{
                        return 0;
                    }
                    
                case wait_leading_space:
                    if(CHECK(duration,leading_space_length) && bit){
                        state = wait_pulse;
                        return 0;
                        
                    }else{
                        reporterror(bit,duration);
                        return 0;
                    }
                case wait_pulse:
                    if(CHECK(duration,pulse_length) && !bit){
                        state = wait_bit;
                        return 0;
                    }else{
                        reporterror(bit,duration);
                        return 0;
                    }
                case wait_bit:{
                    if(!bit ){
                        reporterror(bit,duration);
                        return 0;
                    }

                    state = wait_pulse;
                    byte bit = 0;
                    int result = 0;
                    if (CHECK(duration,space_one_length)) {
                        bit = 1;
                    }
                    bitbuffer = (bitbuffer<<1) | bit;

                   
                    if (bitbuffer & 0b100000000){
                        result = add(reverse_bit(bitbuffer&0xFF));
                        bitbuffer = 1;
                    }
                    return result;
                }
            }
        }
};


/**

17C (0x11) Auto, 19 (0x13) ggf Vorlauf
dump: d2 0c 1c 2d 07 0d a0 4c 9c f1 
dump: dd 11 13 04 00 11 00 00 00 39 

17 Cool
dump: cc 0c 1c 2d 07 0d a0 4c 1c 71 
dump: d2 0c 1c 2d 07 0d a0 4c 1c 71

17 Heat
                           XX
dump: cc 0c 1c 2d 07 0d a0 6c 1c 91 
dump: d2 0c 1c 2d 07 0d a0 6c 1c 91 
dump: dd 11 12 09 00 10 00 00 00 3c 

02:22
dump: cc 0c 1c 2d 07 0d a0 2c 1c 51 
dump: d2 0c 1c 2d 07 0d a0 2c 1c 51 


02:23 ?
dump: d2 0c 1c 2d 07 0d a0 2c 1c 51 
dump: dd 11 11 0e 00 0f 00 00 00 3f 

02:24
dump: dd 12 11 0f 00 0f 00 00 00 41 
dump: dd 11 11 0f 00 0f 00 00 00 40 


02:25?
dump: dd 12 11 0f 00 0f 00 00 00 41 
dump: d2 0c 1c 2d 07 0d a0 6c 1c 91 


02:26
dump: cc 0c 1c 2d 07 0d a0 2c 1c 51 
dump: d2 0c 1c 2d 07 0d a0 2c 1c 51 

02:27
dump: dd 12 11 0f 00 10 00 00 00 42 
dump: d2 0c 1c 2d 07 0d a0 2c 1c 51 


Target Temp:
-------------------
28  9c  0b10011100          1c  0b00011100
27  9b
26  9a

8   88  0b10001000          08  0b00001000


*/