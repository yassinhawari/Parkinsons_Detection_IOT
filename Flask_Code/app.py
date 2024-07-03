from flask import Flask, request, send_file, jsonify, render_template,redirect, url_for
import os
import requests
import time
from utility.process_audio_file import process_audio_file



app = Flask(__name__)
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

ESP32_IP = "http://192.168.137.165"  # Replace with your ESP32 IP address

# Global variable to store vibration data
vibration_data = True

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/start_recording', methods=['GET'])
def start_recording():
    try:
        response = requests.get(f"{ESP32_IP}/start")
        if response.status_code == 200:
            return jsonify({"message": "Recording started", "status": "success"})
        else:
            return jsonify({"message": "Failed to start recording", "status": "error"})
    except Exception as e:
        return jsonify({"message": str(e), "status": "error"})

@app.route('/recording_complete', methods=['GET'])
def recording_complete():
    global vibration_data
    vibration = request.args.get('vibration')
    vibration_data = True if vibration == 'true' else False
    print(vibration_data)
    audio_path = os.path.join(UPLOAD_FOLDER, 'recording.wav')
    
    # Download the audio file from ESP32
    response = requests.get(f"{ESP32_IP}/download", stream=True)
    if response.status_code == 200:
        with open(audio_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=1024):
                if chunk:
                    f.write(chunk)
        prediction_result = process_audio_file(audio_path)
        return redirect(url_for('result'))
    else:
        return jsonify({"message": "Failed to download audio", "status": "error"})

@app.route('/result', methods=['GET'])
def result():
    global vibration_data
    status = process_audio_file("/uploads/recording.wav")
    return render_template('result.html', vibration=vibration_data, status=status)

if __name__ == '__main__':
    app.run(host='0.0.0.0',port=5000)