import numpy as np
import parselmouth
from parselmouth.praat import call
import joblib
import warnings
import os

warnings.filterwarnings("ignore", message="X does not have valid feature names, but MinMaxScaler was fitted with feature names")

def extract_features(audio_path):
    try:
        # Load audio file
        sound = parselmouth.Sound(audio_path)

        # Convert sound object to pitch object for fundamental frequency analysis
        pitch = sound.to_pitch()

        # Convert sound object to a point process for jitter and shimmer analysis
        point_process = call(sound, "To PointProcess (periodic, cc)", 75, 500)

        # Extract frequency features
        mdvp_fo = call(pitch, "Get mean", 0, 0, "Hertz")  # Average vocal fundamental frequency
        mdvp_fhi = call(pitch, "Get maximum", 0, 0, "Hertz", "Parabolic")  # Maximum vocal fundamental frequency
        mdvp_flo = call(pitch, "Get minimum", 0, 0, "Hertz", "Parabolic")  # Minimum vocal fundamental frequency

        # Extract jitter measurements (variation in fundamental frequency)
        jitter_local = call(point_process, "Get jitter (local)", 0, 0, 0.0001, 0.02, 1.3)
        jitter_local_absolute = call(point_process, "Get jitter (local, absolute)", 0, 0, 0.0001, 0.02, 1.3)
        jitter_rap = call(point_process, "Get jitter (rap)", 0, 0, 0.0001, 0.02, 1.3)
        jitter_ppq5 = call(point_process, "Get jitter (ppq5)", 0, 0, 0.0001, 0.02, 1.3)
        jitter_ddp = jitter_rap * 3  # DDP is three times the RAP value

        # Extract shimmer measurements (variation in amplitude)
        shimmer_local = call([sound, point_process], "Get shimmer (local)", 0, 0, 0.0001, 0.02, 1.3, 1.6)
        shimmer_apq3 = call([sound, point_process], "Get shimmer (apq3)", 0, 0, 0.0001, 0.02, 1.3, 1.6)
        shimmer_apq5 = call([sound, point_process], "Get shimmer (apq5)", 0, 0, 0.0001, 0.02, 1.3, 1.6)
        shimmer_apq11 = call([sound, point_process], "Get shimmer (apq11)", 0, 0, 0.0001, 0.02, 1.3, 1.6)
        shimmer_dda = shimmer_apq3 * 3

        # Harmonics-to-Noise Ratio (measure of voice quality)
        harmonicity = sound.to_harmonicity()
        hnr = call(harmonicity, "Get mean", 0, 0)

        # Additional derived features
        sound_values = sound.values[0]
        sound_diff = np.diff(sound_values)

        # Construct the feature vector
        features = [
            mdvp_fo, mdvp_fhi, mdvp_flo,
            jitter_local, jitter_local_absolute, jitter_rap, jitter_ppq5, jitter_ddp,
            shimmer_local, shimmer_apq3, shimmer_apq5, shimmer_apq11, shimmer_dda,
            hnr,
            np.mean(sound_values), np.std(sound_values),
            np.mean(sound_diff), np.std(sound_diff),
            np.var(sound_values), np.log10(np.var(sound_diff)),
            np.exp(-np.std(sound_diff)),
            np.var(sound_diff)
        ]
        return features
    except Exception as e:
        print(f"An error occurred during feature extraction: {e}")
        return None
def process_audio_file(audio_path):
    Extra_Trees = joblib.load('joblib/Extra_Trees.joblib',)
    features = extract_features(audio_path)

    # Check if any features are NaN and handle the situation
    if features is not None and not np.isnan(features).any():
        # Reshape the features for prediction
        features_reshaped = np.array(features).reshape(1, -1)

        try:
            # Predict status for the new data
            print("features", features_reshaped[0])
            prediction = Extra_Trees.predict(features_reshaped)

            # Output the prediction
            status = "Parkinson" if prediction == 1 else "Healthy"
            print("Extracted Features:", features)
            print("="*230)
            print(f"Status prediction for the extracted features: {status}")
            return status
        except Exception as e:
            print("An error occurred during prediction:", e)
    else:
        # If features are NaN, remove the recording and prompt for a new recording
        if os.path.exists(audio_path):
            os.remove(audio_path)
        print("The recording wasn't clear enough. Please try recording your voice again and make sure to speak clearly.")