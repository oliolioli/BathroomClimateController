#include <M5Atom.h>
#include "M5_ENV.h"
#include "webServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Initiate humidity and temp sensors
SHT3X sht30;
QMP6988 qmp6988;

// 110580 Bytes is the maximum ammount of memory (via malloc)
// As we work with 2 and 4 Bytes of information chunks (arranged in arrays), we will work with chunks of 18430 Bytes
#define dataSize 18430
#define minutesOfADay 1440

// Declare variables for further data storage
float* temp; // Declare tmp as pointer to float
unsigned short* hum; // Declare hum as pointer to unsigned short
int currentIndex = 0;  // Keep track of the current index for data storage

// Save the last 7 days
float last7daysTemp[7];
float last7daysHum[7];

void setup() {
    // Start running M5, sensor and access point
    M5.begin(true,false,true);
    Wire.end();
    Wire.begin(26,32);
    qmp6988.init();
    setupAP();

    // Allocate memory for the temperature and humidity data arrays
    temp = (float*) malloc(dataSize * sizeof(float));  // 4 Bytes
    hum = (unsigned short*) malloc(dataSize * sizeof(unsigned short));  // 2 Bytes
    
    // Check if we could allocate the needed memory
    if (temp == NULL || hum == NULL) {
      Serial.println("Failed to allocate memory");
      while (1);  // Halt if failed to allocate memory
    }

    // Mock Sensor Data - this is needed as we would have to run the M5 for about 12 days to fill all the storage
    // After the mocked data is inserted, we begin to fill and work with real data sensor. But to begin with, the whole array is filled once with mocked data
    mockSensorsRandom();

    // Filling the arrays with 0 on the first run (not needed if we work with mocking data)
    // firstRun();

    // Get the mean temp of the last 7 days and save them in an array to pass it to the webserver
    for (int i = 0; i < 7; ++i) {
      last7daysTemp[i] = getTempLast_n_days(i+1);
    }

    // Get the mean temp of the last 7 days and save them in an array to pass it to the webserver
    for (int i = 0; i < 7; ++i) {
      last7daysHum[i] = getHumidityLast_n_days(i+1);
    }
}

// Generates and prints 'count' random numbers in range [lower, upper].
int printRandoms(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

// Mock sensor with random data
void mockSensorsRandom() {
    // Use current time as seed for random generator
    srand(time(0));
    int j = 1;
    int prefix = 1;
    float change = 0;
    
    // Fill both arrays to the fullest (takes ~2min)
    for (int i = 0; i < dataSize; i++) {
      temp[i] = printRandoms(20, 30) + change;  // Set random temp in range (15-30°C)
      hum[i] = printRandoms(29, 30) - (change * printRandoms(0, 2));  // Set random humidity (30-85%)
      
      // We change the prefix every day and thus the direction of the temp curve
      if (j % (2 * minutesOfADay) == 0) {
        prefix = prefix * -1;
      }
      j = j + prefix;      
      
      change = j/300;

      //change = prefix * (10 + round(j / 140 ));
      //change = prefix * 100 * sin(j % minutesOfADay);
      
      //change = sin( minutesOfADay / j);
      Serial.printf("Change: %f\n", change);

      //change = sin(round(j / 140));
      //change = 100 * sin(1 + (j % minutesOfADay));


      if (i == dataSize - 1) {
        Serial.printf("Filled both arrays with randomised mocked sensor data, reached index: %d\n", i);
      }
    }
}


//Fill arrays with 0 when running for the first time - so the graph can be displayed properly.
void firstRun() {
    // Fill both arrays to the fullest (takes ~2min)
    for (int i = 0; i < dataSize; i++) {
      temp[i] = 0;
      hum[i] = 0; 
      if (i == dataSize - 1) {
        Serial.printf("Ran for the first time, successfully filled the arrays with 0s\n");
      }
    }
}

/** 
 * Get mean temperature of whole days (day 1 = 24h back in time, day 2 = 48h-24h back in time, and so on...)
 * Therefore days must be >= 1
 *
 * To get the data, we iterate through our saved sensor data, calculating back in time as we saved data every minute
 */
float getTempLast_n_days(int days) {
  float sumTemp = 0;
  int start = 0;
  int offset = 0;

  // Calculating offsets
  if (currentIndex - (days * minutesOfADay) < 0) {
    offset = abs(currentIndex - (days * minutesOfADay));
    start = dataSize - offset;
  } else {
    start = currentIndex - (days * minutesOfADay);
  }

  // Serial.printf("Starting point to calculate last %d days: %d\n", days, start);

  // Sum up all the values in the specific minutes
  // Starting from start but then just count one day upwards 
  // (because we want the mean of day n and not the mean of day n till day 0)
  for (int i = 0; i < minutesOfADay; ++i) {
    sumTemp += temp[start];

    // Increment the index, wrapping back to 0 when it reaches dataSize
    start = (start + 1) % dataSize;
  }

  // Serial.printf("End point to calculate last %d days: %d\n", days, start);
  Serial.printf("Mean temp last %dth day: %.1f\n", days, sumTemp / minutesOfADay);
  return sumTemp / minutesOfADay;
}

/** 
 * Get mean humidity of whole days (day 1 = 24h back in time, day 2 = 48h-24h back in time, and so on...)
 * Therefore days must be >= 1
 *
 * To get the data, we iterate through our saved sensor data, calculating back in time as we saved data every minute
 */
float getHumidityLast_n_days(int days) {
  float sumHum = 0;
  int start = 0;
  int offset = 0;

  // Calculating offsets
  if (currentIndex - (days * minutesOfADay) < 0) {
    offset = abs(currentIndex - (days * minutesOfADay));
    start = dataSize - offset;
  } else {
    start = currentIndex - (days * minutesOfADay);
  }

  // Serial.printf("Starting point to calculate last %d days: %d\n", days, start);

  // Sum up all the values in the specific minutes
  // Starting from start but then just count one day upwards 
  // (because we want the mean of day n and not the mean of day n till day 0)
  for (int i = 0; i < minutesOfADay; ++i) {
    sumHum += hum[start];
    
    // Increment the index, wrapping back to 0 when it reaches dataSize
    start = (start + 1) % dataSize;
  }

  // Serial.printf("End point to calculate last %d days: %d\n", days, start);
  Serial.printf("Mean humidity last %dth day: %.1f\n", days, sumHum / minutesOfADay);
  return sumHum / minutesOfADay;
}


void loop() {
    
    // Even if we mocked all the data during initialisation, we can now work with real data we get from the sensors
    // So we can display actual temp, humidity and dew point and just have to mock the rest of the data for the plotting

    // Get sensor values
    sht30.get();

    // Set sensor values to current index in specific arrays
    temp[currentIndex] = sht30.cTemp;
    hum[currentIndex] = sht30.humidity;

    if (ledDisplay(temp[currentIndex], hum[currentIndex]) == 1){
      M5.dis.drawpix(0, 0xff0000);  // RED  红色
    } else {
      M5.dis.drawpix(0, 0x00ff00);  // GREEN  绿色
    }
    
    // We save the sensor values every minute
    delay(1);
    
    // And let the webserver run
    loopAP();

    // Pass humidity and temperature to webserver so he can display it
    setHumidity(hum[currentIndex]);
    setTemperature(temp[currentIndex]);
    setDew(temp[currentIndex], hum[currentIndex]);

    //Serial.printf("hum: %d\n",hum[currentIndex]);
    //Serial.printf("temp: %.2f\n",temp[currentIndex]);

    // Increment the index, wrapping back to 0 when it reaches dataSize
    currentIndex = (currentIndex + 1) % dataSize;
}
