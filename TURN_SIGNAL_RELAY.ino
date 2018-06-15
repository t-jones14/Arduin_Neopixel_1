/*
  PURPOSE: This application was built for an automotive application. Client replaced OEM turn signal's with aftermarket multicolor
  LED strips controlled by an unknown controller. In order for the turn signal function of the LED strips to work the
  controller had to be powered on. It was undesired to run the LED's every time the vehicle was driven. Therefore, the
  controller needed to be supplied constant power when an input from the OEM turn signal wiring (which alternates 16V/0V/16V/0V...
  and so-forth was applied.

  OVERVIEW: In a nutshell: this application will supply constant output to any specified output (pin 13 in this case) when 5 Volts is
  applied to any specified Analog input (pin A1 in this case). When voltage to the input pin is discontinued after 250milliseconds
  the constant output will be terminated. The application will then wait for voltage to be resupplied to the input pin.

  This application will also support an alternating input source (turn signal in this case). The amount of time to wait before returning
  the output pin to an OFF state is determined one of two methods.

  METHOD 1: Setting a default by amending the timeoutWait variable.
  METHOD 2: Using the Learn Button to time voltage drops and adjust the time-out wait time accordingly (+ 50 milliseconds for any variations in timing)

  USING THE LEARN BUTTON: At any point in time, press and hold the push button for approximately 6-seconds (real time - non simulation). This will
  time the difference in time between Voltage > 1 and Voltage < 1 and set it as the timeoutWait time. NOTE: The variable timeoutWait is not updated!
  The variable blinkRate is updated with this data.

  CIRCUIT: For emulation purposes a circuit was built using a 5V Power Supply connected to an ATTINY microprocessor (to emulate the alternating
  turn signal of 5V/0V/5V/0V alternation every 250milliseconds) connected to pin A1 on an Arduino UNO R3. An LED with a 1kΩ resistor was connected to pin 13.
  The learn button is powered and grounded from the arduino board with a 10kΩ resistor and connected to Digital Pin 2 on the arduino board.*/

// Constants won't change
int output = 13;            // Pin to output voltage
int input = A1;             // Pin to receive voltage
int learnButton = 2;        // Pin to receive signal for learn mode
int waitTime = 500;         // Time to wait in milliseconds
int timeoutWait = 250;      // Time to wait in milliseconds

// These variables will change
int relayState = LOW;       // Track what state the relay is in
bool timeout = false;       // Track if we're timed-out or not
bool learnMode = false;     // Track if we're supposed to be in learn mode or not
int blinkRate = 0;          // Store blink interval in millis (obtained from learn mode)
int lastButtonState = 0;    // Track the last state of the learn button
int buttonState = 0;        // Track the current state of the learn button

// The following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long timerStart = 0;        // Track when timer started
long timerStop = 0;         // Track when timer stopped
long passedTime = 0;        // Track time elapsed =(timerStop - timerStart)
long blinkStart = 0;        // Track when blink HIGH (learn mode)
long blinkStop = 0;         // Track when blink LOW  (learn mode)

void setup()
{
  pinMode(relayState, OUTPUT); // Set output pin
  pinMode(input, INPUT);       // Set input pin
  pinMode(learnButton, INPUT); // Set input pin (learn mode)
}

void loop()
{
  int sensorValue = analogRead(input);          // Read voltage at input pin
  float voltage = sensorValue * (5.0 / 1023.0); // Convert
  buttonState = digitalRead(learnButton);       // Read the learnButton pin
  Serial.begin(9600);                           //  Begin serial communication

  if (buttonState == HIGH)  // If the learnButton is pressed
  {
    learnMode = true;   // Enter learnMode
  }
  delay(50);        // Debounce

  if (voltage > 1)    // If Voltage is Greater Than 1 Volt
  {
    timeout = false;  // Set timeout state to FALSE since there's no reason to timeout
    passedTime = 0;   // Reset passedTime variable
    timerStart = 0;   // Reset timerStart variable
    timerStop = 0;    // Reset timerStop variable
  }

  if (voltage < 1)  //If Voltage is Less Than 1 Volt
  {
    timerStart = millis();                 // Start the timeout timer
    delay(waitTime);                       // How long should we wait until it times out?
    timerStop = millis();                  // Stop the timeout timer
    passedTime = (timerStop - timerStart); // Calculate how much time we waited

    if (passedTime >= waitTime) //If the calculated time is greater than or equal to our wait time
    {
      sensorValue;      // Re-check the Voltage first

      if (voltage < 1)  // If the voltage is still Less Than 1 Volt
      {
        timeout = true; // Set timeout state to TRUE since the voltage has been less than 1 Volt for >= our Wait Time
        timerStart = 0; // Reseet timerStart variable
        timerStop = 0;  // Reset timerStop variable
        passedTime = 0; // Reset passedTime variable
      }
    }
  }

  while (timeout == true)                    // If TIMEOUT = TRUE
  {
    relayState = LOW;                        // Set the relay state to LOW/OFF
    sensorValue = analogRead(input);         // Read voltage at input pin
    voltage = sensorValue * (5.0 / 1023.0);  // Convert
    digitalWrite(output, relayState);        // Tell the relay to turn off

    if (voltage > 1)    // if the voltage is Greater Than 1 Volt
    {
      timeout = false;  // Set timeout state to FALSE since there's no reason to timeout
      timerStart = 0;   // Reseet timerStart variable
      timerStop = 0;    // Reset timerStop variable
      passedTime = 0;   // Reset passedTime variable

    }

    if (voltage < 1)
    {
      timeout = true; // Set timeout state to TRUE since the voltage has been less than 1 Volt
      timerStart = 0; // Reseet timerStart variable
      timerStop = 0;  // Reset timerStop variable
      passedTime = 0; // Reset passedTime variable
    }
  }

  while (timeout == false)  // If TIMEOUT = FALSE
  {
    sensorValue = analogRead(input);        // Read voltage at input pin
    voltage = sensorValue * (5.0 / 1023.0); // Convert
    relayState = HIGH;                      // Set the relay to ON/HIGH since we're not timed out
    digitalWrite(output, relayState);       // The the relay to turn on
    buttonState = digitalRead(learnButton); // Read the learnButton pin

    if (buttonState == HIGH)  // If the learnButton is pressed
    {
      learnMode = true; // Enter learnMode
    }
    delay(50);        // Debounce

    if (voltage < 1)  // If the voltage is Less Than 1 Volt
    {
      timerStart = millis();                // Start the timeout timer

      if (blinkRate > 100)          // If blinkRate is Greater Than 100
      {
        timeoutWait = blinkRate + 50; // Set timeoutWait to blinkRate value + 50 milliseconds for any variations in timing
      }
      else                // If blinkRate is not Greater Than 100
      {
        timeoutWait = timeoutWait; // Leave default value alone
      }

      delay(timeoutWait);                     // How long should we wait to timeout in this state?
      timerStop = millis();                   // Stop the timeout timer
      passedTime = (timerStop - timerStart);  // Calculate how much time we waited

      if (passedTime >= timeoutWait)  //If the calculated time is greater than or equal to our assigned wait time
      {
        sensorValue;        //Re-check the sensor first

        if (voltage < 1)    //If the sensor read less than 1 Volt
        {
          relayState = LOW; //Set the relay state to OFF/LOW.
          timeout = true;   //Keep us in the timed out state.
          timerStart = 0;   //Reset timerStart variable.
          timerStop = 0;    //Reset timerStop variable.
          passedTime = 0;   //Resed passedTime variable.
        }
      }
    }
  }
  while (learnMode == true)  // If LEARNMODE = TRUE
  {
    sensorValue = analogRead(input);        // Read voltage at input pin
    voltage = sensorValue * (5.0 / 1023.0); // Convert
    Serial.println("LEARN MODE ACTIVE");  // Print to Serial Monitor

    while (voltage > 1)
    {
      sensorValue = analogRead(input);        // Read voltage at input pin
      voltage = sensorValue * (5.0 / 1023.0); // Convert
      blinkStart = millis();          // Set blinkStart as running millis();
      Serial.print("Start: ");          // Print to Serial Monitor
      Serial.println(blinkStart);       // Print to Serial Monitor
      delay(50);                // Debounce
    }
    while (voltage < 1)
    {
      sensorValue = analogRead(input);        // Read voltage at input pin
      voltage = sensorValue * (5.0 / 1023.0); // Convert
      blinkStop = millis();           // Set blinkStop as running millis();
      Serial.print("Stop: ");         // Print to Serial Monitor
      Serial.println(blinkStop);        // Print to Serial Monitor
      delay(50);                // Debounce
    }

    blinkRate = (blinkStop - blinkStart);   // Calculate blinkRate
    Serial.print("Blink Interval: ");     // Print to Serial Monitor
    Serial.println(blinkRate);          // Print to Serial Monitor
    delay(50);                  // Debounce
    learnMode = false;              // Exit learnMode
  }
  digitalWrite(output, relayState);       // Send relayState
}
