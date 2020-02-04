#include <SPI.h>
#include <mcp_can.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
const int spiCSPin = 5;
MCP_CAN CAN(spiCSPin);
const int potPin = 34;
int otk = 0;
int potValue1;



QueueHandle_t queue1;


void setup() {
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_8MHz)){
        Serial.println("CAN BUS init Failed");
        delay(1000);}
        Serial.println("CAN BUS Shield Init OK!");
 

  queue1 = xQueueCreate( 1, sizeof( int ) );

  if(queue1 == NULL){
    Serial.println("Error creating the queue");
  }
  xTaskCreate(
                    taskOne,          
                    "TaskOne",        
                    10000,            
                    NULL,             
                    1,                
                    NULL);            
 
  xTaskCreate(
                    taskTwo,          
                    "TaskTwo",        
                    10000,            
                    NULL,             
                    2,                
                    NULL);            
 
}
 
void loop() {
}
 
void taskOne( void * parameter )
{
    
 while(1){
  otk = analogRead(potPin);
  int brojotkucaja = 30 + otk / 31.5;
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  lcd.begin();
  
  if(xQueueSend(queue1, &brojotkucaja, portMAX_DELAY))
  {
    Serial.printf("Upisao sam broj otkucaja: %d \n", brojotkucaja);
    lcd.clear();
    lcd.backlight();
    lcd.print("Heart rate:");
    lcd.setCursor(0, 1);
    lcd.print(brojotkucaja);

    }
    
  else
  {
    Serial.println("Greska u upisu u queue1");
    }

  delay(200);
 }
 
}
 
void taskTwo( void * parameter)
{
    int element;
    int i = 0;
    while(1){
      if(xQueueReceive(queue1, &element, portMAX_DELAY))
      {
        Serial.printf("Primio sam broj otkucaja: %d \n", element);
      }
      else{
        Serial.println("Greska u prijemu iz queue1");
      }

        unsigned char stmp[1] = {element};
       
        //Serial.println("In loop");
        if ((element <50 || element > 100)){
            CAN.sendMsgBuf(0x43, 0, 1, stmp);
            delay(200);
            Serial.printf("Poslao sam na CAN broj otkucaja: %d \n", stmp[1]);}
        else {
          i++;
        }
        if (i == 20){
          CAN.sendMsgBuf(0x43, 0, 1, stmp);
          delay(200);
          Serial.printf("Poslao sam na CAN broj otkucaja: %d \n", stmp[1]);
          i = 0;}
          }

     }

 
