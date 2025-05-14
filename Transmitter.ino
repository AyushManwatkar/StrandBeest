#include <esp_now.h>
#include <WiFi.h>

#define X_AXIS_PIN 32
#define Y_AXIS_PIN 33
#define SWITCH_PIN 25

uint8_t receiverMacAddress[] = {0xAC,0x67,0xB2,0x36,0x7F,0x28};  //AC:67:B2:36:7F:28

struct PacketData
{
    byte xAxisValue;
    byte yAxisValue;
    byte switchPressed;
};
PacketData data;

int mapAndAdjustJoystickDeadBandValues(int value, bool reverse){
    if (value >= 2200){
        value = map(value, 2200, 4095, 127, 254);
    }else if(value <= 1800){
        value = map(value, 1800, 0, 127, 0);  
    }else{
        value = 127;
    }

    if(reverse){
        value = 254 - value;
    }
    return value;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){
    Serial.print("\r\nLast Packet Send Status:\t ");
    Serial.println(status);
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Message sent" : "Message failed");
}

void setup(){
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }else{
        Serial.println("Succes: Initialized ESP-NOW");
    }

    esp_now_register_send_cb(OnDataSent);
    
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
            
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }else{
        Serial.println("Succes: Added peer");
    } 

    pinMode(SWITCH_PIN, INPUT_PULLUP);   
}
 
void loop() 
{
    data.xAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(X_AXIS_PIN), false);
    data.yAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(Y_AXIS_PIN), false);  
    data.switchPressed = false; 

    if (digitalRead(SWITCH_PIN) == LOW){
        data.switchPressed = true;
    }
    
    esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
    if (result == ESP_OK){
        Serial.println("Sent with success");
    }else{
        Serial.println("Error sending the data");
    }    
    
    if (data.switchPressed == true){
        delay(500);
    }else{
        delay(50);
    }
}