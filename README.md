Parkinson's Disease Detection using ESP32, ADXL362, INMP441, and Flask
Parkinson's Disease is a neurodegenerative disorder affecting millions worldwide, characterized by symptoms such as tremors, stiffness, and impaired balance. Early detection and monitoring of these symptoms are crucial for effective treatment. This project leverages ESP32, ADXL362 accelerometer, INMP441 microphone, and Flask to monitor and analyze symptoms through vibration and audio data.

Overview
The ESP32 microcontroller collects vibration data using the ADXL362 accelerometer and audio data using the INMP441 microphone. These data streams are processed in real-time and transmitted wirelessly to a Flask-based web application. The Flask app provides a user-friendly interface for initiating data collection, processing the received data, and presenting the analysis results to users.

Components Used
ESP32: Powerful microcontroller with Wi-Fi capabilities.
ADXL362: 3-axis digital accelerometer for detecting vibrations.
INMP441: Digital microphone for capturing audio recordings.
Flask: Python web framework for developing the web application.
Prerequisites
Before setting up the project, ensure you have the following prerequisites installed and configured:

PlatformIO: For developing and uploading firmware to ESP32.
Arduino IDE: Alternative to PlatformIO for ESP32 development.
Python 3.x: Required for running the Flask web server.
Git: For cloning the project repository and managing source code versions.
Setup Instructions
ESP32 Setup
Clone the Repository:

bash
Copier le code
git clone https://github.com/your_username/esp32-parkinsons.git
cd esp32-parkinsons
Install PlatformIO:

Follow the PlatformIO Installation Guide.

Upload Firmware:

Open the project in PlatformIO or Arduino IDE.
Connect ESP32 via USB and upload the firmware (main.cpp).
Configure ESP32:

Modify main.cpp to adjust sampling rates, data processing algorithms, and Wi-Fi settings.
Flask Web App Setup
Install Dependencies:

bash
Copier le code
cd Flask_App
pip install -r requirements.txt
Run Flask App:

bash
Copier le code
python app.py
Access the Web Interface:

Open a web browser and go to http://localhost:5000 to interact with the Flask web application.

Usage
Start Recording:

Click "Start Recording" on the Flask web interface to initiate data collection.

Data Transmission:

ESP32 collects audio and vibration data, sending it wirelessly to the Flask server for real-time analysis.

Prediction and Results:

Flask processes the data, runs it through machine learning models or algorithms, and displays prediction results related to Parkinson's Disease symptoms.

Project Structure
/ESP32_Code/: Contains ESP32 firmware code (main.cpp).

main.cpp: Firmware for ESP32 microcontroller.
/Flask_App/: Flask application files.

app.py: Main Flask application script.
templates/: HTML templates for Flask web pages.
static/: Static files (CSS, JavaScript) for enhancing the web interface.
Additional Notes
Adjust vibration thresholds (ADXL362) and audio processing algorithms (INMP441) in main.cpp to optimize sensor readings.
Ensure ESP32 and Flask server are connected to the same Wi-Fi network for seamless data transmission.
Experiment with different machine learning models or signal processing techniques within Flask to enhance prediction accuracy.
Contributors
Your Name: Email@example.com
License
This project is licensed under the MIT License. See the LICENSE file for details.
