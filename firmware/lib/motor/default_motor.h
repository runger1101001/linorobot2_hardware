// Copyright (c) 2021 Juan Miguel Jimeno
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DEFAULT_MOTOR
#define DEFAULT_MOTOR

#include <Arduino.h>
#include <Servo.h> 

#include "motor_interface.h"

#include <SimpleFOC.h>
#include "SimpleFOCDrivers.h"
#include "comms/i2c/I2CCommanderMaster.h"






class Generic2: public MotorInterface
{
    private:
        int in_a_pin_;
        int in_b_pin_;
        int pwm_pin_;

    protected:
        void forward(int pwm) override
        {
            digitalWrite(in_a_pin_, HIGH);
            digitalWrite(in_b_pin_, LOW);
            analogWrite(pwm_pin_, abs(pwm));
        }

        void reverse(int pwm) override
        {
            digitalWrite(in_a_pin_, LOW);
            digitalWrite(in_b_pin_, HIGH);
            analogWrite(pwm_pin_, abs(pwm));
        }

    public:
        Generic2(float pwm_frequency, int pwm_bits, bool invert, int pwm_pin, int in_a_pin, int in_b_pin): 
            MotorInterface(invert),
            in_a_pin_(in_a_pin),
            in_b_pin_(in_b_pin),
            pwm_pin_(pwm_pin)
        {
            pinMode(in_a_pin_, OUTPUT);
            pinMode(in_b_pin_, OUTPUT);
            pinMode(pwm_pin_, OUTPUT);

            if(pwm_frequency > 0)
            {
                analogWriteFrequency(pwm_pin_, pwm_frequency);
            }
            analogWriteResolution(pwm_bits);

            //ensure that the motor is in neutral state during bootup
            analogWrite(pwm_pin_, abs(0));
        }

        void brake() override
        {
            analogWrite(pwm_pin_, 0);
        }
};

class Generic1: public MotorInterface
{
    private:
        int in_pin_;
        int pwm_pin_;

    protected:
        void forward(int pwm) override
        {
            digitalWrite(in_pin_, HIGH);
            analogWrite(pwm_pin_, abs(pwm));
        }

        void reverse(int pwm) override
        {
            digitalWrite(in_pin_, LOW);
            analogWrite(pwm_pin_, abs(pwm));
        }

    public:
        Generic1(float pwm_frequency, int pwm_bits, bool invert, int pwm_pin, int in_pin, int unused=-1): 
            MotorInterface(invert),
            in_pin_(in_pin),
            pwm_pin_(pwm_pin)
        {
            pinMode(in_pin_, OUTPUT);
            pinMode(pwm_pin_, OUTPUT);

            if(pwm_frequency > 0)
            {
                analogWriteFrequency(pwm_pin_, pwm_frequency);
            }
            analogWriteResolution(pwm_bits);

            //ensure that the motor is in neutral state during bootup
            analogWrite(pwm_pin_, abs(0));
        }

        void brake() override
        {
            analogWrite(pwm_pin_, 0);
        }
};

class BTS7960: public MotorInterface
{
    private:
        int in_a_pin_;
        int in_b_pin_;

    protected:
        void forward(int pwm) override
        {
            analogWrite(in_a_pin_, 0);
            analogWrite(in_b_pin_, abs(pwm));
        }

        void reverse(int pwm) override
        {
            analogWrite(in_b_pin_, 0);
            analogWrite(in_a_pin_, abs(pwm));
        }

    public:
        BTS7960(float pwm_frequency, int pwm_bits, bool invert, int unused, int in_a_pin, int in_b_pin): 
            MotorInterface(invert),
            in_a_pin_(in_a_pin),
            in_b_pin_(in_b_pin)
        {
            pinMode(in_a_pin_, OUTPUT);
            pinMode(in_b_pin_, OUTPUT);

            if(pwm_frequency > 0)
            {
                analogWriteFrequency(in_a_pin_, pwm_frequency);
                analogWriteFrequency(in_b_pin_, pwm_frequency);

            }
            analogWriteResolution(pwm_bits);

            //ensure that the motor is in neutral state during bootup
            analogWrite(in_a_pin_, 0);
            analogWrite(in_b_pin_, 0);
        }
    
        BTS7960(float pwm_frequency, int pwm_bits, bool invert, int in_a_pin, int in_b_pin): 
            MotorInterface(invert),
            in_a_pin_(in_a_pin),
            in_b_pin_(in_b_pin)
        {
            pinMode(in_a_pin_, OUTPUT);
            pinMode(in_b_pin_, OUTPUT);

            if(pwm_frequency > 0)
            {
                analogWriteFrequency(in_a_pin_, pwm_frequency);
                analogWriteFrequency(in_b_pin_, pwm_frequency);

            }
            analogWriteResolution(pwm_bits);

            //ensure that the motor is in neutral state during bootup
            analogWrite(in_a_pin_, 0);
            analogWrite(in_b_pin_, 0);
        }

        void brake() override
        {
            analogWrite(in_b_pin_, 0);
            analogWrite(in_a_pin_, 0);            
        }
};

class ESC: public MotorInterface
{
    private:
        Servo motor_;
        int pwm_pin_;

    protected:
        void forward(int pwm) override
        {
            motor_.writeMicroseconds(1500 + pwm);
        }

        void reverse(int pwm) override
        {
            motor_.writeMicroseconds(1500 + pwm);
        }

    public:
        ESC(float pwm_frequency, int pwm_bits, bool invert, int pwm_pin, int unused=-1, int unused2=-1): 
            MotorInterface(invert),
            pwm_pin_(pwm_pin)
        {
            motor_.attach(pwm_pin);
            
            //ensure that the motor is in neutral state during bootup
            motor_.writeMicroseconds(1500);
        }

        void brake() override
        {
            motor_.writeMicroseconds(1500);         
        }
};








class I2CCOMMANDER: public MotorInterface
{
    private:
        

// our RosmoESC's address...
#define TARGET_I2C_ADDRESS 0x60


// global instance of commander - controller device version
I2CCommanderMaster commander;

void setup() {
    // slow start - give RosmoESC a chance to boot up, serial to connect, etc...
    delay(1000);

    // this is a debug setup so initialize and wait for serial connection
    Serial.begin(115200);
    while (!Serial);

    while (!Wire.begin(21, 22, 100000)) {    // standard wire pins for ESP32 PICO Kit v4 
        Serial.println("I2C failed to initialize");
        delay(1000);
    } 
    commander.addI2CMotors(TARGET_I2C_ADDRESS, 1); // only one motor in my test setup
    commander.init();
    Serial.println("I2C Commander intialized.");



    protected:
        void forward(int pwm) override
        {
        (commander.writeRegister(0, REG_TARGET, &targetSpeed, 4)!=4) 
           // old: motor_.writeMicroseconds(1500 + pwm);
        }

        void reverse(int pwm) override
        {
        (commander.writeRegister(0, REG_TARGET, &targetSpeed, -1)!=-1) 
           // old motor_.writeMicroseconds(1500 + pwm);
        }

 //   public:
 //       I2CCOMMANDER(float pwm_frequency, int pwm_bits, bool invert, int pwm_pin, int unused=-1, int unused2=-1): 
 //           MotorInterface(invert),
  //          pwm_pin_(pwm_pin)
  //      {
  //          motor_.attach(pwm_pin);
            
            //ensure that the motor is in neutral state during bootup
  //          motor_.writeMicroseconds(1500);
   //     }

        void brake() override
        {
                (commander.writeRegister(0, REG_TARGET, &targetSpeed, -0)!=-0) 

           // old motor_.writeMicroseconds(1500);         
        }
//};
   
};

#endif






