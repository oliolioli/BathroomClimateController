#include "webServer.h"
#include <M5Atom.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <string.h>

// Taken from https://github.com/m5stack/M5AtomU/blob/master/examples/Advanced/WIFI/WiFiSetting/WebServer.h

// Set these to your desired credentials.
const char *ssid     = "BathroomClimateController";
const char *password = "123456789";

// Already declared and filled in NOMA.ino
extern float last7daysTemp[7];
extern float last7daysHum[7];

// Initiate the tree variables of the sensor data were interested in
unsigned short humidity = 0;
float temperature = 0;
float dewPoint = 0;

// Set up the webserver
WiFiServer server(80);

void setupAP() {
    Serial.println(
        "\nWIFI ACCESS POINT");         // Screen print string
    Serial.printf("Please connect:%s \nThen access to:", ssid);
    WiFi.softAP(
        ssid,
        password);                      // You can remove the password parameter if you want the AP to be open.
    IPAddress myIP = WiFi.softAPIP();   // Get the softAP interface IP address.
    Serial.println(myIP);
    server.begin();                     // Start the established Internet of Things network server.
}

// Three setter-functions for humidity, temperature and dew point (we call them in NOMA.ino)
void setHumidity(unsigned short& hum) {
    humidity = (unsigned short) hum;
    //Serial.printf("hum: %u\n",humidity);
}

void setTemperature(float& temp) {
    temperature = (float) temp;
    //Serial.printf("temp: %.2f\n",temperature);
}

void setDew(float& temp, unsigned short& hum) {
    // dew point is calculated with Magnus-Tetens formula
    dewPoint = (float) temp - ((100 - (unsigned short) hum)/5);
    //Serial.printf("DewPoint: %.2f\n", dewPoint);
}

int ledDisplay(float& temp, unsigned short& hum)
  {
    float dew = (float) temp - ((100 - (unsigned short) hum)/5);

    if (dew > temp){
      Serial.printf("temp great than dew: %.1f > %.1f\n", temp, dew);
      return 1;
    } else {
      return 0;
    }
  }

// Let the webserver run and generate a HTML page (with javascript and html canvas)
void loopAP() {
    WiFiClient client =
        server
            .available();               // listen for incoming clients.

    if (client) {                       // if you get a client.
        Serial.print("New Client:");
        String currentLine =
            "";                         // make a String to hold incoming data from the client.
        while (client.connected()) {    // loop while the client's
                                        // connected,continuously receiving data.
            if (client.available()) {   // if there's bytes to read from the
                                        // client.
                char c =
                    client.read();      // store the read a byte.
                Serial.write(c);
                if (c == '\n') {        // if the byte is a newline character.
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head><style>.shade {border:5px solid #4db8ff; margin-left:10px; padding:5px; background-color: #eef;box-shadow: -6px -6px rgba(0,0,0,0.6);-moz-box-shadow: -6x -6x rgba(0,0,0,0.6);-webkit-box-shadow: -6px -6px rgba(0,0,0,0.6);-o-box-shadow: -6px -6px rgba(0,0,0,0.6);}");
                        client.println("h1 {text-align: margin-left:30px;padding: 10px;margin-top: -45px;font-family: Helvetica, sans-serif;font-size: 50px;transform: skewY(-6deg);letter-spacing: 4px;word-spacing: -8px;color: #4db8ff;text-shadow: -1px -1px 0 firebrick,-2px -2px 0 firebrick,-3px -3px 0 #002080, -4px -4px 0 #002080, -5px -5px 0 #002080, -6px -6px 0 #002080, -7px -7px 0 #002080, -8px -8px 0 #002080, -30px 20px 40px dimgrey}h2 {font-family: Helvetica, sans-serif}</style></head>");
                        client.println("<body>");
                        client.println("");
                        client.println("<h1 style=\"margin-left:25px\">BATHROOM<br>CLIMATE &#128704;<br>CONTROLLER</h1></br>");
                        client.println("<div style=\"width:550px\" class=\"shade\">");
                        // Here the actual sensor data is printed
                        client.printf("<h2>Temperature : %.1f&deg;C</h2>", temperature);
                        client.printf("<h2>Dew Point: %.1f&deg;C</h2>", dewPoint);
                        client.printf("<h2>Humidity: %d%%</h2>", humidity);
                        client.println("</div></br>");
                        client.println("<canvas id=\"myCanvas\" width=\"550\" height=\"450\" class=\"shade\"");
                        client.println("Your browser does not support the canvas element.");
                        client.println("</canvas>");
                        client.println("");
                        // Plot historized data with html canvas
                        client.println("<script>");
                        client.println("function drawCanvas( tempGraph, dewPointGraph, humidityGraph){  ");
                        client.println("    var canvas = document.getElementById( \"myCanvas\" );  ");
                        client.println("    var context = canvas.getContext( \"2d\" );");
                        client.println("    var GRAPH_TOP = 25;  ");
                        client.println("    var GRAPH_BOTTOM = 375;  ");
                        client.println("    var GRAPH_LEFT = 35;  ");
                        client.println("    var GRAPH_RIGHT = 480;    ");
                        client.println("    var GRAPH_HEIGHT = 350;  ");
                        client.println("    var GRAPH_WIDTH = 450;  ");
                        client.println("");
                        client.println("    // get max temp to draw right y-axis (= temp and also dew point)");
                        client.println("    var largestTemp = 0;  ");
                        client.println("    for( var i = 0; i < tempGraph.length; i++ ){  ");
                        client.println("        if( tempGraph[ i ] > largestTemp ){  ");
                        client.println("            largestTemp = tempGraph[ i ] + 5;  ");
                        client.println("        }  ");
                        client.println("    }");
                        client.println("    ");
                        client.println("    // get max humidity to draw left y-axis (=humidity)");
                        client.println("    var largestHumidity = 0;  ");
                        client.println("    for( var i = 0; i < humidityGraph.length; i++ ){  ");
                        client.println("        if( humidityGraph[ i ] > largestHumidity ){  ");
                        client.println("            largestHumidity = humidityGraph[ i ] + 5;  ");
                        client.println("        }  ");
                        client.println("    }");
                        client.println("    context.clearRect( 0, 0, 500, 400 );  ");
                        client.println("    context.font = \"12px Arial\";  ");
                        client.println("       ");
                        client.println("    // draw X and right Y axis  ");
                        client.println("    context.beginPath();  ");
                        client.println("    context.moveTo( GRAPH_LEFT, GRAPH_BOTTOM );  ");
                        client.println("    context.lineTo( GRAPH_RIGHT, GRAPH_BOTTOM );  ");
                        client.println("    context.lineTo( GRAPH_RIGHT, GRAPH_TOP );  ");
                        client.println("    context.stroke();");
                        client.println("    ");
                        client.println("    // draw humidity axis (Y axis on the left)");
                        client.println("    context.beginPath();");
                        client.println("    context.moveTo( GRAPH_LEFT, GRAPH_BOTTOM );  ");
                        client.println("    context.lineTo( GRAPH_LEFT, GRAPH_TOP );");
                        client.println("    context.stroke();");
                        client.println("");
                        client.println("    // draw reference values for humidity (left y-axis)");
                        client.println("    context.beginPath();");
                        client.println("    context.fillStyle = \"#002080\";  ");
                        client.println("    context.fillText(Math.round(largestHumidity) + \"%\", GRAPH_LEFT - 30, GRAPH_TOP);");
                        client.println("    context.stroke();");
                        client.println("    context.fillText(Math.round(largestHumidity / 4) + \"%\", GRAPH_LEFT - 30, ( GRAPH_HEIGHT ) / 4 * 3 + GRAPH_TOP );");
                        client.println("    context.stroke();");
                        client.println("    context.fillText(Math.round(largestHumidity / 2) + \"%\", GRAPH_LEFT - 30, Math.round( GRAPH_HEIGHT ) / 2 + GRAPH_TOP );");
                        client.println("    context.stroke();");
                        client.println("    context.fillText(Math.round(largestHumidity / 4 * 3) + \"%\", GRAPH_LEFT - 30, ( GRAPH_HEIGHT ) / 4 + GRAPH_TOP );");
                        client.println("    context.stroke();");
                        client.println("    ");
                        client.println("    // draw reference values for temp (right y-axis)");
                        client.println("    context.beginPath();");
                        client.println("    context.fillStyle = \"black\";  ");
                        client.println("    context.strokeStyle = \"#BBB\";");
                        client.println("    context.fillText(Number((largestTemp).toFixed(1)) + \" C\xB0\", GRAPH_RIGHT + 15, GRAPH_TOP);");
                        client.println("    context.stroke();");
                        client.println("    ");
                        client.println("    // draw temp reference line #2");
                        client.println("    context.beginPath();  ");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT ) / 4 * 3 + GRAPH_TOP );  ");
                        client.println("    context.lineTo( GRAPH_RIGHT, ( GRAPH_HEIGHT ) / 4 * 3 + GRAPH_TOP );");
                        client.println("    ");
                        client.println("    // draw reference value for temp (right y-axis)");
                        client.println("    context.fillText( Number((largestTemp / 4).toFixed(1)) + \" C\xB0\", GRAPH_RIGHT + 15, ( GRAPH_HEIGHT ) / 4 * 3 + GRAPH_TOP);    ");
                        client.println("    context.stroke();");
                        client.println("      ");
                        client.println("    // draw temp reference line #3");
                        client.println("    context.beginPath();  ");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT ) / 2 + GRAPH_TOP );");
                        client.println("    context.lineTo( GRAPH_RIGHT, ( GRAPH_HEIGHT ) / 2 + GRAPH_TOP );");
                        client.println("    ");
                        client.println("    // draw reference value for temp (right y-axis)");
                        client.println("    context.fillText( Number((largestTemp / 2).toFixed(1)) + \" C\xB0\", GRAPH_RIGHT + 15, Math.round( GRAPH_HEIGHT ) / 2 + GRAPH_TOP);  ");
                        client.println("    context.stroke();  ");
                        client.println("    ");
                        client.println("    // draw temp reference line #4");
                        client.println("    context.beginPath();");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT ) / 4 + GRAPH_TOP );");
                        client.println("    context.lineTo( GRAPH_RIGHT, ( GRAPH_HEIGHT ) / 4 + GRAPH_TOP );");
                        client.println("    ");
                        client.println("    // draw reference value for temp (right y-axis)  ");
                        client.println("    context.fillText( Number((largestTemp / 4 * 3).toFixed(1)) + \" C\xB0\", GRAPH_RIGHT + 15, ( GRAPH_HEIGHT ) / 4 + GRAPH_TOP);  ");
                        client.println("    context.stroke();  ");
                        client.println("    // draw titles  ");
                        client.println("    context.fillText( \"Date\", GRAPH_RIGHT / 2, GRAPH_BOTTOM + 50);  ");
                        client.println("");
                        client.println("    // ### 1. Drawing graph temperature ###");
                        client.println("    context.beginPath();  ");
                        client.println("    context.lineJoin = \"round\";  ");
                        client.println("    context.strokeStyle = \"black\";  ");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT - tempGraph[ 0 ] / largestTemp * GRAPH_HEIGHT ) + GRAPH_TOP );  ");
                        client.println("    // Get day and month for x-axis");
                        client.println("    var dateObj = new Date();");
                        client.println("    var newdate = dateObj.getUTCDate() + \".\" + (dateObj.getUTCMonth() + 1);");
                        client.println("    ");
                        client.println("    // draw reference value for day of the week");
                        client.println("    for( var i = 1; i < tempGraph.length; i++ ){  ");
                        client.println("        context.lineTo( GRAPH_RIGHT / tempGraph.length * i + GRAPH_LEFT, ( GRAPH_HEIGHT - tempGraph[ i ] / largestTemp * GRAPH_HEIGHT ) + GRAPH_TOP );");
                        client.println("");
                        client.println("    // draw reference value for day of the week  ");
                        client.println("    context.fillText(newdate - i, GRAPH_RIGHT - (i * tempGraph.length * 10) , GRAPH_BOTTOM + 25);    ");
                        client.println("    ");
                        client.println("    }");
                        client.println("    context.stroke();  ");
                        client.println("   ");
                        client.println("    // ### 2. Drawing graph dew point ###");
                        client.println("    context.beginPath();  ");
                        client.println("    context.lineJoin = \"round\";  ");
                        client.println("    context.strokeStyle = \"red\";  ");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT - dewPointGraph[ 0 ] / largestTemp * GRAPH_HEIGHT ) + GRAPH_TOP );  ");
                        client.println("    ");
                        client.println("    for( var i = 1; i < dewPointGraph.length; i++ ){  ");
                        client.println("        context.lineTo( GRAPH_RIGHT / dewPointGraph.length * i + GRAPH_LEFT, ( GRAPH_HEIGHT - dewPointGraph[ i ] /  largestTemp * GRAPH_HEIGHT ) + GRAPH_TOP );");
                        client.println("    }");
                        client.println("    context.stroke();");
                        client.println("   ");
                        client.println("    // ### 3. Drawing graph humidity ###");
                        client.println("    context.beginPath();");
                        client.println("    context.setLineDash([5, 3]);");
                        client.println("    context.lineJoin = \"round\";  ");
                        client.println("    context.strokeStyle = \"#002080\";      ");
                        client.println("    context.moveTo( GRAPH_LEFT, ( GRAPH_HEIGHT - humidityGraph[ 0 ] / largestHumidity * GRAPH_HEIGHT ) + GRAPH_TOP );  ");
                        client.println("    ");
                        client.println("    for( var i = 1; i < humidityGraph.length; i++ ){  ");
                        client.println("        context.lineTo( GRAPH_RIGHT / humidityGraph.length * i + GRAPH_LEFT, ( GRAPH_HEIGHT - humidityGraph[ i ] /  largestHumidity * GRAPH_HEIGHT ) + GRAPH_TOP );");
                        client.println("    }");
                        client.println("    context.stroke();    ");
                        client.println("");
                        client.println("    // Drawing graph temperature legend");
                        client.println("    context.font = \"italic 12px Arial\";  ");
                        client.println("    context.fillStyle = \"black\";");
                        client.println("    context.fillText( \"Temperature\", GRAPH_RIGHT, (GRAPH_BOTTOM) + 40);");
                        client.println("    context.fillStyle = \"red\";  ");
                        client.println("    context.fillText( \"Dew point\", GRAPH_RIGHT, (GRAPH_BOTTOM) + 60 );");
                        client.println("    context.fillStyle = \"#002080\";  ");
                        client.println("    context.fillText( \"Humidity\", GRAPH_LEFT - 30, (GRAPH_BOTTOM) + 50 );");
                        client.println("}");
                        client.println("");
                        client.println("\/\/ Graphs");
                        // ********* Pass the sensor values to the plotted graph *******
                        /* Test values (they work!)
                        client.println("var tempGraph = [ 23.8, 23.4, 23.5, 23.5, 22.5, 24.1, 24.0];");
                        client.println("var dewPointGraph = [ 8, 16, 20, 19.4, 21.4, 22.5, 22.5 ];");
                        client.println("var humidityGraph = [60, 70, 80, 86, 74, 91, 65 ];");
                        */
                        // Print the last 7 days of temperature to the plot 
                        client.printf("var tempGraph = [");
                        for (int i=0; i < 7; i++) {
                            client.printf("%.1f", last7daysTemp[i]);
                            // seperate them by commas, but not the last
                            if (i < 6) {
                              client.print(",");
                            }
                        }
                        client.printf("];");
                        // Print the last 7 days of humidity to the plot
                        client.printf("var humidityGraph = [");
                        for (int i=0; i < 7; i++) {
                            client.printf("%1f", last7daysHum[i]);
                            // seperate them by commas, but not the last
                            if (i < 6) {
                              client.print(",");
                            }
                        }
                        client.printf("];");
                        client.println("// calculate dewPoints (taking arrays tempGraph and humidityGraph)");
                        client.println("var dewPointGraph = [];");
                        client.println("");
                        // fill this array with corresponding dewPoints
                        client.println("for (var i=0; i < tempGraph.length; i++) {");
                        client.println("    dewPointGraph[i] = tempGraph[i] - ((100 - humidityGraph[i]) / 5);");
                        client.println("}");
                        client.println("");
                        client.println("// Draw the graphs! :)");
                        client.println("drawCanvas( tempGraph, dewPointGraph, humidityGraph);");
                        client.println("");
                        client.println("");
                        client.println("</script>");
                        client.println("");
                        client.println("</body>");
                        client.println("</html>");
                        client.println("");

                        // break out of the while loop
                        break;
                    } else {                // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                } else if (c !=
                           '\r') {          // if you got anything else but a carriage
                                            // return character.
                    currentLine += c;       // add it to the end of the currentLine.
                }
            }
        }
        client.stop();  // close the connection.
    }
}
