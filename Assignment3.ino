#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <task.h>


#define SIG1 4                              //defining the pin for the led in task 1
#define PULSE1 200                          //setting the length of the pulse in microseconds
#define BREAK 50                            //setting how long the led  is off in microseconds
#define PULSE2 30                           //settting the length of the second pulse in microseconds

#define FREQUENCYPIN1 5                     //setting the pin that will measure the first frequency of the signal generator
#define FREQUENCYPIN2 6                     //setting hte pin that will measure the second frequency from the signal generator

#define ANALOGUEIN 0                        //defining the pin which will read the analogue input
#define ERRORLED 8                          //defining the pin that will out put the error signal to the  error led

#define FRAMELENGTH 2                       //defining the length of the frame in mmilliseconds

#define CONTROLLED 10                       //defines the pin for the LED in the LED control function
#define PUSHSWITCH 11                       //defines the pin used in push switch task


QueueHandle_t EVENTQUEUE;                   //creates a queue
SemaphoreHandle_t MUTEX;                    //creates a semaphore using the freeRTOS structure 

unsigned long FREQUENCY1 = 0;               //intiating the frequency value for task 2
unsigned long FREQUENCY2 = 0;               // initiating the frequency value for task 3
int FREQ1SB = 0;                            //initiating the value of the scale and bound version of the frequency of task 2
int FREQ2SB = 0;                            //initiating the value of the scale and bound version of the frequency of task 3
int MAXFREQ = 1000;                         // defining the maximum frequency allowed for task 5
int MINFREQ1 = 333;                         //defining the minimum acceptable frequncy to be measured during task 2
int MINFREQ2 = 500;                        //defining the minimum acceptable frequncy to be measured during task 2

unsigned int TOTAL = 0;                    //initialising variables to be used in task 4 to alow the calculation of the average ana;ouge input using the total
unsigned int AVERAGE = 0;
unsigned int RANGE = 3500;                 //range set to 3500 to allow for demonstration of error LED turning on when it is activated
unsigned int ANALOUGE = 0;      
unsigned int i = 0;

unsigned int PULSEHIGH = 0;               //initialising the variable used to calculate the frequency for the frequency generatoe
unsigned int PULSELOW = 0;
float PULSETOTAL = 0;
unsigned int M = 1000000;

unsigned FRAMECOUNT = 0;                  //initialising the frame count variable to be used in the scgedule

struct FrequencyData(){                   //creates a global structure to store the frequenices found in task 2 and task 3 and the scale and bounded result in task 5
  int FREQ1;
  int FREQ2;
  int SB1;
  int SB2;
}

struct FrequencyData FREQUENCY;

void setup() {
  Serial.begin(9600);                     //setting the seral bud rate to 9600

  pinMode(SIG1, OUTPUT);                  //setting the the pin for task 1 to output 

  pinMode(FREQUENCYPIN1, INPUT);          //setting the pin used in task 2 to input
  pinMode(FREQUENCYPIN2, INPUT);          //setting the pin used in task 2 to input

  pinMode(ANALOGUEIN, INPUT);             //setting the pin used to measure the analouge signal in task 4 to input
  pinMode(ERRORLED, OUTPUT);              //setting the pin used in task 4 to out put to allow the use of the erroe led

  pinMode(CONTROLLED, OUTPUT);
  pinMode(PUSHSWITCH, INPUT);
  
  xTaskCreate(DebounceSwitch,             //creates the debounce task and sets is priority for the freeRTOS programs
              "debounceSwitch",
              128,
              NULL,
              3,
              NULL);

  xTaskCreate(LedControl,
              "LedControl",
              128,
              NULL,
              0,
              NULL); 

  xTaskCreate(Task1,
              "Task1",
              128,
              NULL,
              0,
              NULL); 

  xTaskCreate(Task2,
              "Task2",
              128,
              NULL,
              2,
              NULL);    

  xTaskCreate(Task3,
              "Task3",
              128,
              NULL,
              2,
              NULL); 

 xTaskCreate(Task4,
              "Task4",
              128,
              NULL,
              0,
              NULL); 

  xTaskCreate(Task5,
              "Task5",
              128,
              NULL,
              0,
              NULL); 

  MUTEX = xSemaphoreCreateMutex();

  vTaskStartScheduler();

}

void Task1() {                        

  while(1){
  digitalWrite(SIG1, HIGH);               //setting the pin out put high to turn the led on            
  delayMicroseconds(PULSE1);              //keeping the led on to for the length defined in task 1
  digitalWrite(SIG1, LOW);                //setting the pin out put low to turn the led off
  delayMicroseconds(BREAK);               //keeping the led off to for the length defined in task 1
  digitalWrite(SIG1, HIGH);              //setting the pin out put high to turn the led on 
  delayMicroseconds(PULSE2);             //keeping the led on to for the second length defined in task 1 
  digitalWrite(SIG1, LOW);               //turning the led off for a final time
  vTaskDelay(4 /portTICK_PERIOD_MS);
  }
}

void Task2() {

  xSemaphoreTake(MUTEX);


  PULSEHIGH = pulseIn(FREQUENCYPIN1, HIGH);    //determines the amount of time the signal is hig in micro seconds
  PULSELOW = pulseIn(FREQUENCYPIN1, LOW);      //determines the amount of time the signal is low in micro seconds

  PULSETOTAL = PULSEHIGH + PULSELOW;           //calcuates the period of the wave

  FREQUENCY.FREQ1 = M/PULSETOTAL;                   //calculates the frequency in hertz
  xSemaphoreGive(MUTEX);
  vTaskDelay(20 / portTICK_PERIOD_MS);
}

void Task3() {
  xSemaphoreTake(MUTEX);

  PULSEHIGH = pulseIn(FREQUENCYPIN2, HIGH);   //determines the amount of time the signal is hig in micro seconds
  PULSELOW = pulseIn(FREQUENCYPIN2, LOW);     //determines the amount of time the signal is low in micro seconds

  PULSETOTAL = PULSEHIGH + PULSELOW;          //calculates the period of the wave

  FREQUENCY.FREQ1 = M/PULSETOTAL;                  //calculates the frequency in hertz
  xSemaphoreGive(MUTEX);
  vTaskDelay(8 / portTICK_PERIOD_MS);
  
}

void Task4() {

  if (i < 4) {                              //this section of code determines how many times the analouge signal has been recorded 
    TOTAL = TOTAL + ANALOUGE;               //and once the number reaches four it adds up the total and averages it then prints this average in the serial monitor
    ANALOUGE = analogRead(ANALOGUEIN);      //the total is the set back to 0
    i++;
  } else if (i = 4) {
    i = 0;
    AVERAGE = TOTAL / 4;
    TOTAL = 0;
  
  }

  if (AVERAGE > RANGE) {                  //this code determines wheter the average execeeds the range and if it does an led is turned on
    digitalWrite(ERRORLED, HIGH);
  } else {
    digitalWrite(ERRORLED, LOW);
  }
  vTaskDelay(20 / portTICK_PERIOD_MS);
}

void Task5() {

  if(FREQUENCY1 >= 1000){                                           //the code in this section determines weather or not the frequncy for tasks 2 and 3 exceeds their limits 
    FREQ1SB = 99;                                                   //if it does the when the frequncy goes over its limits it is normalised to 99 and when it is under its normalised to 0
  }
  else if(FREQUENCY1 <= 333){
    FREQ1SB = 0;
  }
  else{
    FREQ1SB = ((FREQUENCY1 - MINFREQ1)/(MAXFREQ - MINFREQ1))* 100; //this calculates the normalised frequency for and frequency thiat is within the limits
  }
  if(FREQUENCY2 >= 1000){
    FREQ2SB = 99;
  } 
  else if(FREQUENCY2 <= 500){
    FREQ2SB = 0;
  }
  else{
    FREQ2SB = ((FREQUENCY2 - MINFREQ2)/(MAXFREQ - MINFREQ2))* 100;
  }

  Serial.print(FREQ1SB);                                          //this setiom prints the results in the serial monitor in the formant "%d","%d"
  Serial.print(",");
  Serial.println(FREQ2SB);
  vTaskDelay(100 / portTICK_PERIOD_MS);

}

void DebounceSwitch(){


  vTaskDelay(20 / portTICK_PERIOD_MS);
}

void LedControl(){
  if(){
    digitalWrite(SIG1, HIGH);              //setting the pin out put high to turn the led on 
  }else if{
  digitalWrite(SIG1, LOW); 
  }

}
void loop() {               


  
}