/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID    serviceUUID("46a970e0-0d5f-11e2-8b5e-0002a5d5c51b");
static BLEUUID    HeartRateserviceUUID("180D");
// The characteristic of the remote service we are interested in.
static BLEUUID    SPO2CharacterUUID("0aad7ea0-0d60-11e2-8e3c-0002a5d5c51b");
// static BLEUUID    PPGCharacterUUID("ee0a883a-4d24-11e7-b114-b2f933d5fe66");
static BLEUUID    PulseIntervalCharacterUUID("34e27863-76ff-4f8e-96f1-9e3993aa6199");
static BLEUUID    HeartRateCharacterUUID("2A37");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

static BLERemoteCharacteristic* SPO2RemoteCharacteristic;
static BLERemoteCharacteristic* PulseIntervalRemoteCharacteristic;
static BLERemoteCharacteristic* HeartRateRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData,size_t length,bool isNotify) {
    // Serial.print("Notify callback for characteristic ");
    // Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    // Serial.print(" of data length ");
    // Serial.println(length);
    if(pBLERemoteCharacteristic->getUUID().toString() == SPO2CharacterUUID.toString()){
      Serial.print("SPO2: ");
      Serial.println(pData[7]);
    }else if(pBLERemoteCharacteristic->getUUID().toString() == PulseIntervalCharacterUUID.toString()){
      Serial.print("Pulse Interval: ");
      Serial.println(pData[5]);
    }else if(pBLERemoteCharacteristic->getUUID().toString() == HeartRateCharacterUUID.toString()){
      Serial.print("Heart Rate: ");
      Serial.println(pData[1]);
    }
 
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

class MySecurity : public BLESecurityCallbacks {

	uint32_t onPassKeyRequest(){
    Serial.print("onPassKeyRequest ");
		return 123456;
	}
	void onPassKeyNotify(uint32_t pass_key){
    Serial.print("onPassKeyNotify ");
    ESP_LOGE(LOG_TAG, "The passkey Notify number:%d", pass_key);
	}
	bool onConfirmPIN(uint32_t pass_key){
    Serial.print("onConfirmPIN ");
    ESP_LOGI(LOG_TAG, "The passkey YES/NO number:%d", pass_key);
	  vTaskDelay(5000);
		return true;
	}
	bool onSecurityRequest(){
    Serial.print("onSecurityRequest ");
		ESP_LOGI(LOG_TAG, "Security Request");
		return true;
	}
	void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
	{
    ESP_LOGI(LOG_TAG, "remote BD_ADDR:");
    Serial.print("address type = ");
    Serial.print(auth_cmpl.addr_type);
		Serial.print(" pair status = ");
		Serial.println(auth_cmpl.success);
	}
};



bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  
    // BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	  // BLEDevice::setSecurityCallbacks(new MySecurity());

    // BLESecurity *pSecurity = new BLESecurity();
    // pSecurity->setKeySize();
    // pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
	  // pSecurity->setCapability(ESP_IO_CAP_IO);
	  // pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    delay(3000);
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    BLERemoteService* HeartRateRemoteService = pClient->getService(HeartRateserviceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }else if(HeartRateRemoteService == nullptr){
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service: ");

    connected = true;

    


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    SPO2RemoteCharacteristic = pRemoteService->getCharacteristic(SPO2CharacterUUID);
    PulseIntervalRemoteCharacteristic = pRemoteService->getCharacteristic(PulseIntervalCharacterUUID);
    HeartRateRemoteCharacteristic = HeartRateRemoteService->getCharacteristic(HeartRateCharacterUUID);

    if(connectCharacteristic(pRemoteService, SPO2RemoteCharacteristic) == false)
      connected = false;
    else if(connectCharacteristic(pRemoteService, PulseIntervalRemoteCharacteristic) == false)
      connected = false;
    else if(connectCharacteristic(HeartRateRemoteService, HeartRateRemoteCharacteristic) == false)
      connected = false;

    if(connected == false) {
      pClient-> disconnect();
      Serial.println("At least one characteristic UUID not found");
      return false;
    }

    // if (SPO2RemoteCharacteristic == nullptr) {
    //   Serial.print("Failed to find our characteristic UUID: ");
    //   Serial.println(SPO2CharacterUUID.toString().c_str());
    //   pClient->disconnect();
    //   return false; 
    // }
    // Serial.print(" - Found our characteristic ");
    // Serial.println(SPO2CharacterUUID.toString().c_str());
    
    // Read the value of the characteristic.
    // if(SPO2RemoteCharacteristic->canRead()) {
    //   Serial.println(" can read ");
    //   // std::string value = SPO2RemoteCharacteristic->readValue();
    //   // Serial.print("The characteristic value was: ");
    //   // Serial.println(value.c_str());
    // }

    // if(SPO2RemoteCharacteristic->canNotify())
    //   SPO2RemoteCharacteristic->registerForNotify(notifyCallback);

   
    return true;
}

bool connectCharacteristic(BLERemoteService* pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (l_BLERemoteChar == nullptr) {
    Serial.print("Failed to find one of the characteristics");
    Serial.print(l_BLERemoteChar->getUUID().toString().c_str());
    return false;
  }
  Serial.println(" - Found characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));

  if(l_BLERemoteChar->canNotify())
    l_BLERemoteChar->registerForNotify(notifyCallback);

  return true;
}

bool connectService(BLEClient*  pClient, BLERemoteService* pRemoteService) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find one of the characteristics");
    Serial.print(pRemoteService->getUUID().toString().c_str());
    return false;
  }
  Serial.println(" - Found service: " + String(pRemoteService->getUUID().toString().c_str()));

  return true;
}





/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Serial.print("BLE Advertised Device found: ");
    // Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    // SPO2RemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop
