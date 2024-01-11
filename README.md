# BathroomClimateController: IOT sensor tracking humidity and temperature of your bathroom to avoid mold

![alt text](https://github.com/oliolioli/BathroomClimateController/blob/main/titleImage.png)

_Basic Idea_: As it was advised last winter to reduce heating if possible, mold become a problem. Thus the idea of this project is to sensor:

**1) Humidity**

**2) Temperature**

These two values will be then set off against each other in some sort of simple physical model to know if the room (e.g. bathroom) is too humid or too cold and to take what measure (e.g. increase ventilation or heating).

The mentioned values will be retrievable through a web server running on the sensor device itself, plotting the collected data.

Ideally the bathroom climate control unit enables you to prevent mold but allows you at the same time to save energy as it balances these two goals.


## How to set everything up ##

1. Set 

> https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

In your Arduino **"File" → "Preferences" → "Additional Boards Manager URLs"**

2. Get all the files from git with _git pull_

3. Connect Sensor to your computer and choose Board by "Tools" → "Board" → "M5Stack" → "M5Stack-ATOM"

4. In Arduino click CTRL+Shift+M to display the Serial Monitor. There you have to set Bauds = 600 to see the output of the program.


## Sensors ##

Sensor **ENV III** https://shop.m5stack.com/collections/m5-sensor/products/env-iii-unit-with-temperature-humidity-air-pressure-sensor-sht30-qmp6988 fullfills the requirements and is quite cheap.


## Data handling and storage ##

### Space considerations for stack implementations ###
Arduino says after compilation:

```
Sketch uses 751733 bytes (57%) of program storage space. Maximum is 1310720 bytes.
Global variables use 44448 bytes (13%) of dynamic memory, leaving 283232 bytes for local variables. Maximum is 327680 bytes.
```

We can get exactly **110580 Bytes** on the whole M5 by malloc. Therefore these 110580 Bytes have to be split up into two arrays. As **size of float = 4 Bytes** and **size of unsigned short = 2 Bytes** one array to store data of 2 and the other to store data of 4 Bytes:

110580 Bytes / 6 = 18430 Bytes. Thus array one (2B) gets 2 * 18430 and array two (4B) gets 4 * 18430. 

Check: 2 Bytes * 18430 + 4 Bytes * 18430 = 110580 Bytes ✓

```
// Allocate memory for the temperature and humidity data arrays
  temp = (float*) malloc(dataSize * sizeof(float));  // 4 Bytes
  hum = (unsigned short*) malloc(dataSize * sizeof(unsigned short));  // 2 Bytes
```

## Data structure ## 

In order to work with the sensor data, it must be stored in an accessible data structure. To analyze the available memory space to store the sensor data, we used the function malloc() and determined the available memory of 110’580 Bytes by means of trials. As data types we chose for the percentage value of the humidity unsigned short (2 bytes of memory needed per value) and for the temperature float (4 bytes of memory needed per value). Subsequently, using this information, we calculated that we can store 18’430 measuring points. The needed memory space for two separate arrays of this size is allocated during initialization using the malloc() function:

```
temp = ( float *) malloc ( dataSize * sizeof ( float ) ) ; // 4 Bytes
hum = ( unsigned short *) malloc ( dataSize * sizeof ( unsigned short ) ) ;
// 2 Bytes
```

In order to be able to work with the sensor data over a longer period of time, it is not sufficient
to store data once in the data structure explained above. Based on the decision to store a value
every second, the 18’000+ positions can hold about 12 days of data without renewal. To have
current data beyond these 12 days we decided to overwrite the array from the beginning. To
do this, in each loop in which data is stored, a variable currentindex is increased by one until
the size of the array is reached:

```
currentIndex = ( currentIndex + 1) % dataSize ;
```

This data structure therefore allows us to store values over a certain time and with continuous renewal despite limited memory.

### Traversals ###
We iterate through these arrays with **currentIndex** - set back to zero after reaching the tail (by modulo). By this the data is refreshed automatically and doesn't need to be popped or pushed, as it is a queue. The size limit cannot be removed because we're working with the maximum of available space.


## Access point and webserver ##
It can at times be difficult to make an accessible user-interface on a microcontroller. However the versatility of the Arduino M5 Stack allows to efficiently address the issue. He is indeed capable of hosting a WiFi access point and also a webserver. In other terms, it means that the M5 Stack can answer to requests, in this case HTTP GET requests, sent on the local area network. Not only does it result in not needing to have an external webserver, it also makes it easier and more secure for the user to access the interface as they only have to load the web page after connecting to the WiFi. When starting the device, a new WiFi network called BathroomClimateController will appear. This network can be momentarily joined with password: ”123456789”. Upon connected to the WiFi network, the webpage findable at 192.168.4.1 will display all the relevant information. The device’s LED will automatically turn red if the environment conditions are prone to mold and green otherwise.


## Generating HTML and Plotting ##
JavaScript libraries nowadays eases plotting. But due to their complexity and interconnectedness it is utterly impossible to use them on an offline IOT sensor. Hence we plotted temperature, humidity and corresponding dew point on a grid with HTML Canvas. Humidity and temperature needed two different y-axes and appropriate scaling. As sensor data is saved every minute we could average seven days back in time and could use JavaScript date functions of the local client to print a meaningful timeline on to the x-axis without need of any internal clock on the M5 ATOM. The plotting itself is done by iterating through the preprocessed data, drawing the plot pixel per pixel with the Canvas functions moveTo(), lineTo() and finally stroke().


## Testing and Mocking ##
As there is only one sensor available, we can **mock sensor data** with random temperature and humidity. With this data filled in our storage arrays, we can calculate and send (mocked) data to the webserver and display it.

### Mocking sensor data ###

To ensure that our program works, we have included three different tests. As a basic test we created a Makefile to check the correct compilation of the .cpp files. For example, spelling errors, missing brackets or wrong assignments can be detected directly. Furthermore, pipelines are comprised of jobs, which define what will be done, such as compiling or testing code, as well as stages that spell out when to run the jobs. An example would be running tests after stages that compile the code. Additionally, we used mocking data to simulate the functioning of the entire program. For that purpose we generated randomised temperature and humidity values and filled the whole data structure. Thereby we could test that the data structure runs as designed and improve the graphical representation. The actual sensor data furthermore continuously overwrites the mocked data structure and thus generates real data without need to switch between mocking and real data.

```
// Generates and prints 'count' random numbers in range [lower, upper].
int printRandoms(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}
```

```
// *** Mock sensor ***
  // Use current time as seed for random generator
  srand(time(0));
  
  // Set random temp
  temp[currentIndex] = printRandoms(16, 32);  

  // Set random humidity
  hum[currentIndex] = printRandoms(50, 100);
  // *** End of mocking ***
```
Of course temperature and humidity aren't this random, but we can fill our array in short time and then do calculations on it.

## Webserver and webpage ##
With the includes: <WiFi.h>, <WiFiClient.h> and <WiFiAP.h> we let run a little webserver and an access point on the M5.

When running, a connection can be made to Hotspot: **BathroomClimateAssistant** with password = '123456789'. Then there should be a website under 192.168.4.1 (works better on a pc than on my smartphone - if anyone has this problem).

The webserver serves a single, quite simple HTML page, which is generated by simple "println"-lines in file **webServer.cpp**. To convert valid HTML to this special version, you can use the following script to append a pre- and a postfix to every line (don't forget to escape the double quotes first: '"' has to become '\"').


```
sed -e 's/^/client.println("/' index-escaped.html > index-escaped-prefix.html

```

```
sed -e 's/$/");/' index-escaped-prefix.html > index-escaped-prefix-postfix.html

```




**Technical Problems**
- https://support.arduino.cc/hc/en-us/articles/360016495679-Fix-port-access-on-Linux
