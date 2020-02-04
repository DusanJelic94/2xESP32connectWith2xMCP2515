
#include <SPI.h>
#include <mcp_can.h>
struct packet {
  bool  tmp1;
  bool  tmp2;
};

packet tmp_packet= {false, false};

uint8_t button1 = 12;
uint8_t button2 = 27;
const int spiCSPin = 5;
int INA = 25;
int INB = 26;
MCP_CAN CAN(spiCSPin);
QueueHandle_t queue1;



void setup() {

    pinMode(INA,OUTPUT); 
    pinMode(INB,OUTPUT);
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_8MHz))
    {
        Serial.println("CAN BUS Init Failed");
        delay(100);
    }
    Serial.println("CAN BUS  Init OK!");
    pinMode(button1, INPUT_PULLUP);
    pinMode(button2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(button1), button1_ISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(button2), button2_ISR, FALLING);

    Serial.println("MOtor radi");
    
    queue1 = xQueueCreate( 1, sizeof( uint8_t ) );
    if(queue1 == NULL){
      Serial.println("Error creating the queue");
  }
  xTaskCreate(
                    taskOne,          
                    "TaskOne",        
                    10000,            
                    NULL,             
                    2,                
                    NULL);            
 
  xTaskCreate(
                    taskTwo,          
                    "TaskTwo",        
                    10000,            
                    NULL,             
                    1,                
                    NULL);            

}


void loop() {
}
void button1_ISR(){
  tmp_packet.tmp1 = true;
  digitalWrite(INA,HIGH);
  digitalWrite(INB,LOW);
 }

void button2_ISR(){
  tmp_packet.tmp2 = true;
  digitalWrite(INA,LOW);
  digitalWrite(INB,LOW);
 }

void taskOne( void * parameter )
{   uint8_t len = 0;
    uint8_t buf;
    uint8_t otk; 
    unsigned long canId;
    
 while(1){

  delay(1);
    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
        CAN.readMsgBuf(&len, &buf);

        canId = CAN.getCanId();

        
        otk = buf;       
        if(xQueueSend(queue1, &otk, portMAX_DELAY))
        {
          Serial.printf("Upisao sam broj otkucaja: %d \n", otk);
          
          }
        else
        {
          Serial.println("Greska u upisu u queue1");
          }
       }
    }
 }

void taskTwo( void * parameter)
{
    uint8_t tasterON=0;
    uint8_t tasterOFF=0;  
    uint8_t element;
    while(1){
      
      if(xQueueReceive(queue1, &element, portMAX_DELAY))
      {
        Serial.printf("Primio sam broj otkucaja iz queue: %d \n", element);
        if (tmp_packet.tmp1 == true && tmp_packet.tmp2 == false){
            tasterON = 1;
            tasterOFF = 0;
   
            }
        if (tmp_packet.tmp1 == false && tmp_packet.tmp2 == true){
            tasterON = 0;
            tasterOFF = 1;
            
            }
        if (element > 45 && element < 120 && tasterON == 1 && tasterOFF == 0){
            digitalWrite(INA,HIGH);
            digitalWrite(INB,LOW);
            Serial.printf("MOTOR U RADNOM REŽIMU \n");
            Serial.printf("tasterON = %d \n tasterOFF = %d \n",tasterON,tasterOFF);
            }
        else{
            digitalWrite(INA,LOW);
            digitalWrite(INB,LOW);
            Serial.printf("MOTOR UGAŠEN \n");
            Serial.printf("tasterON = %d \n tasterOFF = %d \n",tasterON,tasterOFF);
            }
        tmp_packet.tmp1 = false;
        tmp_packet.tmp2 = false;
       }
        
    }
      
}


