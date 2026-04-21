from fastapi import FastAPI
from pydantic import BaseModel
import joblib
import pandas as pd
import numpy as np

app = FastAPI()

# -------------------------------
# Load trained model + scaler + columns
# -------------------------------
try:
    model = joblib.load("log_model.pkl")
    scaler = joblib.load("scaler.pkl")
    trained_columns = joblib.load("columns.pkl")
except Exception as e:
    print(f"Model loading error: {e}")
    model, scaler, trained_columns = None, None, None

# -------------------------------
# Load patient dataset - FIXED: correct filename + correct index column
# -------------------------------
try:
    patients_df = pd.read_csv("hospital_readmission_dataset (1).csv")
    patients_df["patient_id"] = patients_df["patient_id"].str.strip().str.upper()
    patients_df.set_index("patient_id", inplace=True)
    print(f"Loaded {len(patients_df)} patients successfully.")
except Exception as e:
    print(f"CSV loading error: {e}")
    patients_df = pd.DataFrame()

# -------------------------------
# Request schema - FIXED to match actual dataset columns
# -------------------------------
class PatientData(BaseModel):
    patient_id: str | None = None
    age: int
    gender: str
    season: str = "Spring"
    region: str = "South"
    primary_diagnosis: str = "Diabetes"
    comorbidities_count: int = 0
    length_of_stay: int = 1
    treatment_type: str = "Medical"
    medications_count: int = 0
    followup_visits_last_year: int = 0
    prev_readmissions: int = 0
    insurance_type: str = "Medicare"
    discharge_disposition: str = "Home"

# -------------------------------
# Helper: encode and predict
# -------------------------------
CATEGORICAL_COLS = ['season', 'gender', 'region', 'primary_diagnosis',
                    'treatment_type', 'insurance_type', 'discharge_disposition']

def predict_from_row(row_df: pd.DataFrame) -> float:
    encoded = pd.get_dummies(row_df, columns=CATEGORICAL_COLS, drop_first=True)
    aligned = encoded.reindex(columns=trained_columns, fill_value=0)
    scaled = scaler.transform(aligned)
    prob = model.predict_proba(scaled)[0][1] * 100
    return round(float(prob), 2)

def get_risk_level(risk: float) -> str:
    if risk >= 70:
        return "HIGH RISK"
    if risk >= 40:
        return "MODERATE RISK"
    return "LOW RISK"

# -------------------------------
# Prediction endpoint
# -------------------------------
@app.post("/predict")
def predict_risk(data: PatientData):
    if model is None or scaler is None or trained_columns is None:
        return {"error": "Model not loaded"}

    row = pd.DataFrame([{
        "age": data.age,
        "gender": data.gender,
        "season": data.season,
        "region": data.region,
        "primary_diagnosis": data.primary_diagnosis,
        "comorbidities_count": data.comorbidities_count,
        "length_of_stay": data.length_of_stay,
        "treatment_type": data.treatment_type,
        "medications_count": data.medications_count,
        "followup_visits_last_year": data.followup_visits_last_year,
        "prev_readmissions": data.prev_readmissions,
        "insurance_type": data.insurance_type,
        "discharge_disposition": data.discharge_disposition,
    }])

    risk_prob = predict_from_row(row)
    return {"risk": risk_prob, "details": data.dict()}

# -------------------------------
# Patient lookup endpoint - FIXED
# -------------------------------
@app.get("/patient/{patient_id}")
def get_patient(patient_id: str):
    if patients_df.empty:
        return {"error": "Patient dataset not loaded"}

    patient_id = patient_id.strip().upper()

    if patient_id not in patients_df.index:
        return {"error": f"Patient '{patient_id}' not found"}

    record = patients_df.loc[patient_id].to_dict()

    actual_score = record.get("readmission_risk_score")
    if pd.notna(actual_score):
        record["historical_risk_score"] = round(float(actual_score) * 100, 2)
    else:
        record["historical_risk_score"] = None

    # Auto-predict using actual patient data
    if model is not None and scaler is not None and trained_columns is not None:
        feature_keys = [c for c in patients_df.columns
                        if c not in ['admission_date', 'label', 'readmission_risk_score']]
        row = pd.DataFrame([{k: record[k] for k in feature_keys}])
        try:
            risk = predict_from_row(row)
            record["predicted_risk"] = risk
            record["risk_level"] = get_risk_level(risk)
            record["status"] = record["risk_level"]
            record["risk_visualization"] = {
                "predicted_risk": risk,
                "historical_risk_score": record["historical_risk_score"],
                "threshold": 40.0
            }
        except Exception as e:
            record["predicted_risk"] = None
            record["prediction_error"] = str(e)

    return record

# -------------------------------
# Feature importance endpoint
# -------------------------------
@app.get("/feature_importance")
def feature_importance():
    if model is not None and trained_columns is not None:
        importance = model.coef_[0]
        feat_df = pd.DataFrame({
            "Feature": trained_columns,
            "Importance": importance
        }).sort_values(by="Importance", ascending=False).head(10)
        return feat_df.to_dict(orient="records")
    return {"error": "Model not loaded"}
