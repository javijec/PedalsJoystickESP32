//#define DEBUG_MODE

#include "SimRacing.h"
#include "USBHIDGamepad.h"
#include "HX711.h"
#include "ST7789_Graphics.h"
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUIDs para Bluetooth LE (Perfil UART)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Definición de pines como constantes en tiempo de compilación
// Definición de pines para Waveshare ESP32-S3-LCD-1.47
static constexpr int Pin_Gas = 4;
static constexpr int Pin_Clutch = 5;
static constexpr int LOADCELL_DOUT_PIN = 2;
static constexpr int LOADCELL_SCK_PIN = 3;
static constexpr int Pin_Brake = -1; // Usamos HX711, no pin analógico

// Constantes para los cálculos
static constexpr float ADC_brake = 16384.0f;
static constexpr int ADC_Max = 4095;
static constexpr uint8_t CHANGE_THRESHOLD = 2;

// Dirección inicial en la EEPROM para los valores de calibración
static constexpr int EEPROM_CALIBRATION_START = 0;
static constexpr uint32_t CALIBRATION_MAGIC = 0x43414C49; // "CALI" en hex
static constexpr int EEPROM_MAGIC_ADDRESS = 0;

// Valores por defecto para la calibración
static constexpr int16_t DEFAULT_GAS_MIN = 0;
static constexpr int16_t DEFAULT_GAS_MAX = 4095;
static constexpr int16_t DEFAULT_BRAKE_MIN = 0;
static constexpr int16_t DEFAULT_BRAKE_MAX = 16384;
static constexpr int16_t DEFAULT_CLUTCH_MIN = 0;
static constexpr int16_t DEFAULT_CLUTCH_MAX = 4095;
static constexpr float DEFAULT_BRAKE_MAX_FORCE = 1000000.0f;

// Estructura para mantener el estado de los pedales
struct PedalState {
    int16_t value;
    bool changed;
} __attribute__((packed));

// Estructura para los valores de calibración
struct CalibrationValues {
    int16_t min;
    int16_t max;
} __attribute__((packed));

// Estructura para todos los valores de calibración
struct AllCalibrationValues {
    uint32_t magic;  // Para verificar si hay calibración válida
    CalibrationValues gas;
    CalibrationValues brake;
    CalibrationValues clutch;
    float brakeMaxForce;  // Nuevo campo para la fuerza máxima del freno
    uint8_t gasDZLow, gasDZHigh;
    uint8_t brakeDZLow, brakeDZHigh;
    uint8_t clutchDZLow, clutchDZHigh;
} __attribute__((packed));

// Wrapper para compatibilidad con la librería Joystick nativa de ESP32-S3
class JoystickWrapper {
private:
    USBHIDGamepad usbJoy;
    int8_t _rx = 0, _ry = 0, _z = 0; // Almacenamos valores para envío en bloque
    
    // Función para mapear valores de pedales (0..Max) a Rango Joystick 8-bit (-127..127)
    int8_t mapJoystick(int32_t v, int32_t vMax) {
        if (vMax == 0) return -127;
        return (v * 254 / vMax) - 127;
    }

public:
    void begin(bool autoSend = true) { usbJoy.begin(); }
    void setZAxisRange(int min, int max) {} 
    void setRxAxisRange(int min, int max) {}
    void setRyAxisRange(int min, int max) {}
    
    // Gas -> Ry (Right Trigger)
    void setRyAxis(int16_t v) { _ry = mapJoystick(v, ADC_Max); }
    // Brake -> Rx (Left Trigger)
    void setRxAxis(int16_t v) { _rx = mapJoystick(v, (int32_t)ADC_brake); }
    // Clutch -> Z (Right Stick Z)
    void setZAxis(int16_t v) { _z = mapJoystick(v, ADC_Max); }
    
    void sendState() { 
        usbJoy.rightTrigger(_ry);
        usbJoy.leftTrigger(_rx);
        usbJoy.rightStick(_z, 0);
    }
};

// Clase para manejar los pedales
class PedalManager {
private:
    SimRacing::ThreePedals& pedals;
    HX711& brake_pedal;
    JoystickWrapper& joystick;
    Preferences preferences;
    
    PedalState gas{0, false};
    PedalState brake{0, false};
    PedalState clutch{0, false};
    
    AllCalibrationValues calibration;
    float brake_scaling_factor;  // Factor de escalado dinámico
    
    void calibratePedal(const char* pedalName, CalibrationValues& calib) {
        Serial.print("Calibrando ");
        Serial.println(pedalName);
        
        Serial.println("No presiones el pedal y presiona Enter");
        while (!Serial.available()) { delay(10); }
        Serial.read();
        
        // Leer valor mínimo (pedal sin presionar)
        if (strcmp(pedalName, "Freno") == 0) {
            float rawValue = brake_pedal.get_value();
            calib.min = (int16_t)rawValue;
            Serial.print("Valor raw mínimo del freno: ");
            Serial.println(rawValue);
        } else {
            int pin = strcmp(pedalName, "Gas") == 0 ? Pin_Gas : Pin_Clutch;
            calib.min = analogRead(pin);
        }
        
        Serial.println("Presiona completamente el pedal y presiona Enter");
        while (!Serial.available()) { delay(10); }
        Serial.read();
        
        // Leer valor máximo (pedal presionado)
        if (strcmp(pedalName, "Freno") == 0) {
            float maxValue = 0;
            Serial.println("Manteniendo presionado el freno, tomando muestras...");
            for(int i = 0; i < 20; i++) {
                float currentValue = brake_pedal.get_value();
                maxValue = max(maxValue, currentValue);
                delay(100);
            }
            calibration.brakeMaxForce = maxValue;
            brake_scaling_factor = ADC_brake / calibration.brakeMaxForce;
            calib.max = ADC_brake;
        } else {
            int pin = strcmp(pedalName, "Gas") == 0 ? Pin_Gas : Pin_Clutch;
            calib.max = analogRead(pin);
        }
    }

    void saveCalibration() {
        calibration.magic = CALIBRATION_MAGIC;
        preferences.begin("pedals", false);
        preferences.putBytes("calib", &calibration, sizeof(AllCalibrationValues));
        preferences.end();
    }

    bool loadCalibration() {
        preferences.begin("pedals", true);
        size_t len = preferences.getBytes("calib", &calibration, sizeof(AllCalibrationValues));
        preferences.end();
        
        if (len != sizeof(AllCalibrationValues) || calibration.magic != CALIBRATION_MAGIC) {
            resetToDefaults();
            return false;
        }
        brake_scaling_factor = ADC_brake / calibration.brakeMaxForce;
        applyCalibration();
        return true;
    }

    void applyCalibration() {
        pedals.setCalibration(
            {calibration.gas.min, calibration.gas.max},
            {calibration.brake.min, calibration.brake.max},
            {calibration.clutch.min, calibration.clutch.max}
        );
    }

    // Variables Bluetooth 
    BLEServer *pServer = NULL;
    BLECharacteristic *pTxCharacteristic;
    bool deviceConnected = false;
    
    // Buffer para Serial print y JSON
    char printBuffer[256]; // Aumentado para seguridad
    
    void sendData(const char* data) {
        // Enviar por USB Serial
        Serial.print(data);
        
        // Enviar por Bluetooth si hay conexión
        if (deviceConnected) {
            pTxCharacteristic->setValue((uint8_t*)data, strlen(data));
            pTxCharacteristic->notify();
        }
    }
    
public:
    void sendJsonState() {
        // Formato: {"g":val, "b":val, "c":val}
        snprintf(printBuffer, sizeof(printBuffer), 
                "{\"g\":%d,\"b\":%d,\"c\":%d}\n", 
                gas.value, brake.value, clutch.value);
        sendData(printBuffer);
    }

    void sendJsonCalibration() {
        // Formato para sincronizar la web: 
        snprintf(printBuffer, sizeof(printBuffer),
                "{\"cal\":{\"gmin\":%d,\"gmax\":%d,\"gdzl\":%d,\"gdzh\":%d,\"bmax\":%.0f,\"bdzl\":%d,\"bdzh\":%d,\"cmin\":%d,\"cmax\":%d,\"cdzl\":%d,\"cdzh\":%d}}\n",
                calibration.gas.min, calibration.gas.max, calibration.gasDZLow, calibration.gasDZHigh,
                calibration.brakeMaxForce, calibration.brakeDZLow, calibration.brakeDZHigh,
                calibration.clutch.min, calibration.clutch.max, calibration.clutchDZLow, calibration.clutchDZHigh);
        sendData(printBuffer);
    }

    int16_t applyDeadzone(int32_t value, int32_t max_val, uint8_t dzLowPct, uint8_t dzHighPct) {
        int32_t lowLimit = (max_val * dzLowPct) / 100;
        int32_t highLimit = max_val - (max_val * dzHighPct) / 100;
        
        if (value <= lowLimit) return 0;
        if (value >= highLimit) return max_val;
        
        // Escalar el valor restante
        return (int16_t)((value - lowLimit) * (int32_t)max_val / (highLimit - lowLimit));
    }

    PedalManager(SimRacing::ThreePedals& p, HX711& b, JoystickWrapper& j) 
        : pedals(p), brake_pedal(b), joystick(j) {}

    // Callbacks para eventos BLE
    class MyServerCallbacks: public BLEServerCallbacks {
        PedalManager* _manager;
    public:
        MyServerCallbacks(PedalManager* m) : _manager(m) {}
        void onConnect(BLEServer* pServer) { _manager->deviceConnected = true; };
        void onDisconnect(BLEServer* pServer) { 
            _manager->deviceConnected = false; 
            BLEDevice::startAdvertising(); // Reiniciar publicidad para permitir nueva conexión
        }
    };

    class MyCallbacks: public BLECharacteristicCallbacks {
        PedalManager* _manager;
    public:
        MyCallbacks(PedalManager* m) : _manager(m) {}
        void onWrite(BLECharacteristic *pCharacteristic) {
            String rxValue = pCharacteristic->getValue();
            if (rxValue.length() > 0) {
                if (rxValue[0] == '{') {
                    _manager->handleJsonCommand(rxValue.c_str());
                } else {
                    _manager->handleSimpleCommand(rxValue[0]);
                }
            }
        }
    };

    void runHardwareDiagnostics() {
        Serial.println("\n--- DIAGNÓSTICO DE HARDWARE ---");
        
        // 1. Verificar HX711 (Freno)
        Serial.print("HX711 (Freno): ");
        if (brake_pedal.is_ready()) {
            Serial.println("OK (Listo)");
            Serial.print("  > Valor Raw actual: ");
            Serial.println(brake_pedal.read());
        } else {
            Serial.println("ERROR (No responde)");
            Serial.println("  > Verifica pines: DOUT=" + String(LOADCELL_DOUT_PIN) + ", SCK=" + String(LOADCELL_SCK_PIN));
            Serial.println("  > Asegúrate de que el HX711 tenga alimentación (VCC/GND)");
        }
        
        // 2. Verificar Analógicos (Gas y Embrague)
        Serial.print("Gas (Pin " + String(Pin_Gas) + "): ");
        Serial.println(analogRead(Pin_Gas));
        Serial.print("Embrague (Pin " + String(Pin_Clutch) + "): ");
        Serial.println(analogRead(Pin_Clutch));
        
        Serial.println("-------------------------------\n");
    }

    void init() {
        display.begin(80);
        display.clearScreen(BLACK);
        display.drawCenteredText(20, "PEDALERA ESP32-S3", GREEN, BLACK, 2);
        display.drawCenteredText(50, "Iniciando...", WHITE, BLACK, 1);

        pedals.begin();
        brake_pedal.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
        brake_pedal.tare(10);
        
        joystick.begin(true);
        loadCalibration();
        updateAll();
        
        // Inicializar Bluetooth LE
        BLEDevice::init("PedalMaster BLE");
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks(this));
        
        BLEService *pService = pServer->createService(SERVICE_UUID);
        pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
        pTxCharacteristic->addDescriptor(new BLE2902());
        
        BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
        pRxCharacteristic->setCallbacks(new MyCallbacks(this));
        
        pService->start();
        pServer->getAdvertising()->start();
        
        sendJsonCalibration();
        
        display.clearScreen(BLACK);
        drawUI();
    }

    void drawUI() {
        display.drawRect(10, 30, 300, 20, DARKGRAY); // Gas
        display.drawRect(10, 70, 300, 20, DARKGRAY); // Brake
        display.drawRect(10, 110, 300, 20, DARKGRAY); // Clutch
        display.drawText(10, 15, "GAS", WHITE);
        display.drawText(10, 55, "BRAKE", WHITE);
        display.drawText(10, 95, "CLUTCH", WHITE);
    }

    void updateScreen() {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate < 50) return;
        lastUpdate = millis();

        display.drawProgressBar(10, 30, 300, 20, gas.value, ADC_Max, GREEN, BLACK, DARKGRAY);
        display.drawProgressBar(10, 70, 300, 20, brake.value, ADC_brake, RED, BLACK, DARKGRAY);
        display.drawProgressBar(10, 110, 300, 20, clutch.value, ADC_Max, BLUE, BLACK, DARKGRAY);
        sendJsonState();
    }

    void resetToDefaults() {
        calibration.gas = {DEFAULT_GAS_MIN, DEFAULT_GAS_MAX};
        calibration.brake = {DEFAULT_BRAKE_MIN, DEFAULT_BRAKE_MAX};
        calibration.clutch = {DEFAULT_CLUTCH_MIN, DEFAULT_CLUTCH_MAX};
        calibration.brakeMaxForce = DEFAULT_BRAKE_MAX_FORCE;
        calibration.gasDZLow = 2; calibration.gasDZHigh = 2;
        calibration.brakeDZLow = 2; calibration.brakeDZHigh = 2;
        calibration.clutchDZLow = 2; calibration.clutchDZHigh = 2;
        calibration.magic = CALIBRATION_MAGIC;
        brake_scaling_factor = ADC_brake / calibration.brakeMaxForce;
        applyCalibration();
        saveCalibration();
    }

    void startCalibration() {
        calibratePedal("Gas", calibration.gas);
        calibratePedal("Freno", calibration.brake);
        calibratePedal("Embrague", calibration.clutch);
        applyCalibration();
        saveCalibration();
        sendJsonCalibration();
    }

    inline bool checkChange(PedalState& state, int16_t newValue) {
        if (abs(newValue - state.value) > CHANGE_THRESHOLD) {
            state.value = newValue;
            state.changed = true;
            return true;
        }
        state.changed = false;
        return false;
    }

    void updateGas() {
        int16_t newValue = pedals.getPosition(SimRacing::Gas, 0, ADC_Max);
        newValue = applyDeadzone(newValue, ADC_Max, calibration.gasDZLow, calibration.gasDZHigh);
        if (checkChange(gas, newValue)) joystick.setRyAxis(gas.value);
    }

    void updateBrake() {
        int32_t raw_value = brake_pedal.get_value();
        int16_t scaledValue = constrain(raw_value * brake_scaling_factor, 0, ADC_brake);
        int16_t newValue = applyDeadzone(scaledValue, ADC_brake, calibration.brakeDZLow, calibration.brakeDZHigh);
        if (checkChange(brake, newValue)) joystick.setRxAxis(brake.value);
    }

    void updateClutch() {
        int16_t newValue = pedals.getPosition(SimRacing::Clutch, 0, ADC_Max);
        newValue = applyDeadzone(newValue, ADC_Max, calibration.clutchDZLow, calibration.clutchDZHigh);
        if (checkChange(clutch, newValue)) joystick.setZAxis(clutch.value);
    }

    void updateAll() {
        pedals.update();
        updateGas();
        updateBrake();
        updateClutch();
        updateScreen();
        if (gas.changed || brake.changed || clutch.changed) joystick.sendState();
    }

    void handleSimpleCommand(char command) {
        switch(command) {
            case 'c': startCalibration(); break;
            case 'r': resetToDefaults(); break;
            case 'm': sendJsonCalibration(); break;
            case 'd': runHardwareDiagnostics(); break;
        }
    }

    void handleJsonCommand(const char* json) {
        if (strstr(json, "\"cmd\":\"setDZ\"")) {
            char pedal = ' ';
            int low = 2, high = 2;
            const char* pPtr = strstr(json, "\"p\":\"");
            if (pPtr) pedal = pPtr[5];
            const char* lowPtr = strstr(json, "\"low\":");
            if (lowPtr) low = atoi(lowPtr + 6);
            const char* highPtr = strstr(json, "\"high\":");
            if (highPtr) high = atoi(highPtr + 7);
            
            if (pedal == 'g') { calibration.gasDZLow = (uint8_t)low; calibration.gasDZHigh = (uint8_t)high; }
            else if (pedal == 'b') { calibration.brakeDZLow = (uint8_t)low; calibration.brakeDZHigh = (uint8_t)high; }
            else if (pedal == 'c') { calibration.clutchDZLow = (uint8_t)low; calibration.clutchDZHigh = (uint8_t)high; }
            saveCalibration();
            sendJsonCalibration();
        }
    }
};

SimRacing::ThreePedals pedals(Pin_Gas, Pin_Brake, Pin_Clutch);
HX711 brake_pedal;
JoystickWrapper Joystick;
PedalManager pedalManager(pedals, brake_pedal, Joystick);

void setup() {
    Serial.begin(115200);
    pedalManager.init();
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() > 0) {
            if (input.startsWith("{")) {
                pedalManager.handleJsonCommand(input.c_str());
            } else {
                pedalManager.handleSimpleCommand(input[0]);
            }
        }
    }
    pedalManager.updateAll();
}