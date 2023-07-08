#include <ArduinoBLE.h>

#define BLE_LED_PIN D2

static const char* greeting = "Salutations";
unsigned long tester = 0;
BLEService greetingService("180C");  // User defined service

BLEStringCharacteristic greetingCharacteristic("2A56",  // standard 16-bit characteristic UUID
    BLERead, 13); // remote clients will only be able to read this

void setup() {
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin
  pinMode(BLE_LED_PIN,OUTPUT);
  digitalWrite(BLE_LED_PIN,LOW);

  if (!BLE.begin()) {   // initialize BLE
    Serial.println("starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("Nano33BLE");  // Set name for connection
  BLE.setAdvertisedService(greetingService); // Advertise service
  greetingService.addCharacteristic(greetingCharacteristic); // Add characteristic to service
  BLE.addService(greetingService); // Add service
  greetingCharacteristic.setValue(greeting); // Set greeting string

  BLE.advertise();  // Start advertising
  Serial.print("Peripheral device MAC: ");
  Serial.println(BLE.address());
  Serial.println("Waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();  // Wait for a BLE central to connect

  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central MAC: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()){
      greetingCharacteristic.writeValue("Hello World");
      // tester += 1;
    } // keep looping while connected
    
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

// #include <ArduinoBLE.h>

// BLEService dataService("180C"); // Define the service UUID
// BLECharacteristic dataCharacteristic("2A56", BLERead, 13); // Define the characteristic UUID and permissions

// void setup() {
//   Serial.begin(9600);
//   while (!Serial); // Wait for serial connection
//   if (!BLE.begin()) {
//     Serial.println("Failed to initialize BLE!");
//     while (1);
//   }
//   BLE.setLocalName("Breathalyzer");
//   BLE.setAdvertisedService(dataService); // Advertise the data service
//   dataService.addCharacteristic(dataCharacteristic); // Add the data characteristic to the service
//   BLE.addService(dataService); // Add the service
//   dataCharacteristic.setValue("Hello, World!"); // Set an initial value for the characteristic
//   BLE.advertise(); // Start advertising the service
//   Serial.println("BLE service started!");
// }

// void loop() {
//   if (BLE.connected()) {
//     // Send data to the central device
//     int sensorValue = 5;
//     String sensorValueStr = String(sensorValue);
//     dataCharacteristic.writeValue(sensorValueStr.c_str(), sensorValueStr.length());
//   }
//   delay(1000); // Wait for one second before sending the next sensor value
// }