#include <driver/i2s.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <FS.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <ADXL362.h>

const char* ssid = "redmi10";
const char* password = "123456789";
const char* flaskServerIP = "http://192.168.137.28:5000"; // Change <Flask-Server-IP> to your Flask server IP address

WebServer server(80);

// Calibration offsets (based on your provided values)
const float xOffset = -120;
const float yOffset = -104;
const float zOffset = 1148;

// Threshold for detecting vibration (g-force change)
const float vibrationThreshold = 0.15; // Adjust this value based on sensitivity requirements

// Previous acceleration values
float prevX = 0;
float prevY = 0;
float prevZ = 0;

#define I2S_WS 15
#define I2S_SD 13
#define I2S_SCK 2
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (16 * 1024)
#define RECORD_TIME       (5) // Seconds
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

// ADXL362 pins configuration
#define ADXL362_CS_PIN 5
#define ADXL362_SCK_PIN 18
#define ADXL362_MISO_PIN 19
#define ADXL362_MOSI_PIN 23
#define VIBRATION_THRESHOLD 200

// ADXL362 object
ADXL362 adxl;

bool detectVibration(int16_t x, int16_t y, int16_t z);

File file;
const char filename[] = "/recording.wav";
const int headerSize = 44;

void SPIFFSInit();
void i2sInit();
void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len);
void i2s_adc();
void example_disp_buf(uint8_t *buf, int length);
void wavHeader(byte *header, int wavSize);
void listSPIFFS(void);
void handleRoot();
void handleDownload();
void handleStartRecording();
void notifyFlaskRecordingComplete(bool vibrationDetected);
bool detectVibration(float x, float y, float z);
bool checkVibrationForSeconds(int seconds);

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  SPIFFSInit();
  i2sInit();
  SPI.begin(ADXL362_SCK_PIN, ADXL362_MISO_PIN, ADXL362_MOSI_PIN, ADXL362_CS_PIN);
  adxl.begin(ADXL362_CS_PIN);
  adxl.beginMeasure();

  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.on("/start", handleStartRecording);

  server.begin();
  Serial.println("HTTP server started.");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}

void SPIFFSInit() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    while (1) yield();
  }
  listSPIFFS();
}

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 64,
    .dma_buf_len = 1024,
    .use_apll = 1
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void i2s_adc_data_scale(uint8_t * d_buff, uint8_t* s_buff, uint32_t len) {
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 2) {
    dac_value = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048;
  }
}

void i2s_adc() {
  int i2s_read_len = I2S_READ_LEN;
  int flash_wr_size = 0;
  size_t bytes_read;
  char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));
  uint8_t* flash_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));

  i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
  i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

  Serial.println(" *** Recording Start *** ");
  while (flash_wr_size < FLASH_RECORD_SIZE) {
    i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, i2s_read_len);
    file.write((const byte*) flash_write_buff, i2s_read_len);
    flash_wr_size += i2s_read_len;
    ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
    ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
  }

  free(i2s_read_buff);
  free(flash_write_buff);
  file.close();
    int16_t rawX, rawY, rawZ;
  // Variables to hold the calibrated acceleration values
  float x, y, z;

  // Read raw acceleration values
  rawX=adxl.readXData();
  rawY=adxl.readYData();
  rawZ=adxl.readZData();

  // Convert raw values to g-force
  x = (rawX - xOffset) * 0.001; // Assuming scale factor of 1 mg/LSB
  y = (rawY - yOffset) * 0.001;
  z = (rawZ - zOffset) * 0.001;

  // Print calibrated acceleration values to the serial monitor
  Serial.print("X: "); Serial.print(x); Serial.print(" g ");
  Serial.print("Y: "); Serial.print(y); Serial.print(" g ");
  Serial.print("Z: "); Serial.print(z); Serial.println(" g");

  // Check for vibration
  bool isVibrating = checkVibrationForSeconds(2);

  // Print vibration detection result
  if (isVibrating) {
    Serial.println("Vibration detected!");
  } else {
    Serial.println("No vibration.");
  }

  prevX = x;
  prevY = y;
  prevZ = z;
  delay(500);
  notifyFlaskRecordingComplete(isVibrating);
  vTaskDelete(NULL);
  listSPIFFS();}

void notifyFlaskRecordingComplete(bool vibrationDetected)
  {
    HTTPClient http;
    String url = String(flaskServerIP) + "/recording_complete?vibration=" + (vibrationDetected ? "true" : "false");
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
      Serial.println("HTTP Response code: " + String(httpResponseCode));
    } else {
    Serial.println("Error on sending request: " + String(http.errorToString(httpResponseCode).c_str()));
  }
  http.end();
}

void handleRoot() {
  server.send(200, "text/html", "Welcome to ESP32 Web Server");
}

void handleDownload() {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "audio/wav");
  file.close();
}

void handleStartRecording() {
  SPIFFS.remove(filename);
  file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    server.send(500, "text/plain", "Failed to create file");
    return;
  }

  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);
  file.write(header, headerSize);

  xTaskCreate([](void*) { i2s_adc(); }, "i2s_adc", 1024 * 8, NULL, 1, NULL);

  server.send(200, "text/plain", "Recording started");
}

void wavHeader(byte* header, int wavSize) {
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 16;
  header[17] = 0;
  header[18] = 0;
  header[19] = 0;
  header[20] = 1;
  header[21] = 0;
  header[22] = 1;
  header[23] = 0;
  header[24] = (byte)(I2S_SAMPLE_RATE & 0xFF);
  header[25] = (byte)((I2S_SAMPLE_RATE >> 8) & 0xFF);
  header[26] = (byte)((I2S_SAMPLE_RATE >> 16) & 0xFF);
  header[27] = (byte)((I2S_SAMPLE_RATE >> 24) & 0xFF);
  unsigned int byteRate = I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8;
  header[28] = (byte)(byteRate & 0xFF);
  header[29] = (byte)((byteRate >> 8) & 0xFF);
  header[30] = (byte)((byteRate >> 16) & 0xFF);
  header[31] = (byte)((byteRate >> 24) & 0xFF);
  header[32] = I2S_SAMPLE_BITS / 8;
  header[33] = 0;
  header[34] = I2S_SAMPLE_BITS;
  header[35] = 0;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);
}

void listSPIFFS(void) {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}

bool checkVibrationForSeconds(int seconds) {
  unsigned long startTime = millis();
  bool vibrationDetected = false;

  while (millis() - startTime < seconds * 1000) {
    // Variables to hold the raw acceleration values
    int16_t rawX, rawY, rawZ;
    // Variables to hold the calibrated acceleration values
    float x, y, z;

    // Read raw acceleration values
    rawX=adxl.readXData();
    rawY=adxl.readYData();
    rawZ=adxl.readZData();

    // Convert raw values to g-force
    x = (rawX - xOffset) * 0.001; 
    y = (rawY - yOffset) * 0.001;
    z = (rawZ - zOffset) * 0.001;

    // Check for vibration
    if (detectVibration(x, y, z)) {
      vibrationDetected = true;
    }

    // Update previous values
    prevX = x;
    prevY = y;
    prevZ = z;

    // Delay between readings
    delay(100);
  }

  return vibrationDetected;
}

bool detectVibration(float x, float y, float z) {
  float deltaX = abs(x - prevX);
  float deltaY = abs(y - prevY);
  float deltaZ = abs(z - prevZ);
  Serial.println(deltaX);
  return (deltaX > vibrationThreshold || deltaY > vibrationThreshold );
}