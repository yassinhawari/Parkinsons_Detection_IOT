Introduction
Parkinson's Disease is a neurodegenerative disorder that affects millions worldwide, characterized by symptoms such as tremors, stiffness, and impaired balance. Early detection and monitoring of these symptoms are crucial for effective treatment and management of the disease. This project leverages the capabilities of ESP32, ADXL362 accelerometer, and INMP441 microphone to monitor and analyze symptoms through vibration and audio data.

The ESP32 microcontroller is programmed to collect vibration data using the ADXL362 accelerometer and audio data using the INMP441 microphone. These data streams are processed in real-time and transmitted wirelessly to a Flask-based web application. The Flask app provides a user-friendly interface for initiating data collection, processing the received data, and presenting the analysis results to users.

Components Used
ESP32: A powerful microcontroller widely used in IoT applications, chosen here for its processing power and Wi-Fi capabilities.
ADXL362: A high-resolution, 3-axis digital accelerometer known for its ultra-low power consumption and high sensitivity, ideal for detecting subtle vibrations associated with Parkinson's symptoms.
INMP441: A high-performance, low-power digital microphone capable of capturing clear audio recordings, essential for analyzing speech patterns and other auditory symptoms.
Flask: A lightweight web framework for Python, used to develop the web application that interfaces with the ESP32 and displays the results to users.
Prerequisites
Before setting up the project, ensure you have the following prerequisites installed and configured:

PlatformIO: PlatformIO is recommended for developing and uploading firmware to the ESP32. Installation instructions can be found here.
Arduino IDE: Alternatively, you can use the Arduino IDE for ESP32 development if preferred over PlatformIO.
Python 3.x: Required for running the Flask web server and managing dependencies.
Git: To clone the project repository and manage source code versions.
Setup Instructions
Follow these detailed instructions to set up and deploy the ESP32 firmware and Flask web application:

ESP32 Setup
Clone the Repository:

bash
Copier le code
git clone https://github.com/your_username/esp32-parkinsons.git
cd esp32-parkinsons
Install PlatformIO:

If not installed, follow the PlatformIO Installation Guide.
Upload Firmware:

Open the project in PlatformIO or Arduino IDE.
Connect ESP32 via USB and upload the firmware (main.cpp).
Configure ESP32:

Modify main.cpp to adjust sampling rates, data processing algorithms, and Wi-Fi settings as per your requirements.
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

On the Flask web interface, click "Start Recording" to initiate data collection.
ESP32 begins recording audio through INMP441 and detecting vibrations using ADXL362.
Data Transmission:

ESP32 sends collected data wirelessly to the Flask server for real-time analysis.
Prediction and Results:

Flask processes the received data, runs it through machine learning models or algorithms, and displays prediction results related to Parkinson's Disease symptoms.
Project Structure
/ESP32_Code/: Contains ESP32 firmware code (main.cpp).

main.cpp: Firmware for ESP32 microcontroller, handles data acquisition from ADXL362 and INMP441, and transmission to Flask server.
/Flask_App/: Flask application files (app.py, templates/, static/).

app.py: Main Flask application script, defines routes, and handles data processing and visualization.
templates/: HTML templates for Flask web pages.
static/: Static files (CSS, JavaScript) for enhancing the web interface.
Additional Notes
Adjust vibration thresholds (ADXL362) and audio processing algorithms (INMP441) in ESP32 firmware (main.cpp) to optimize sensor readings based on your environment and application requirements.
Ensure ESP32 and Flask server are connected to the same Wi-Fi network for seamless data transmission and real-time analysis.
Experiment with different machine learning models or signal processing techniques within Flask to enhance prediction accuracy and symptom detection capabilities.
Contributors
Your Name: Email@example.com
License
This project is licensed under the MIT License. See the LICENSE file for details.
