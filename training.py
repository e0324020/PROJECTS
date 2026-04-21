import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import confusion_matrix, classification_report, roc_auc_score
import joblib


df = pd.read_csv("hospital_readmission_dataset (1).csv")




raw_data = df.copy()  

df_clean = df.drop(['patient_id', 'admission_date', 'readmission_risk_score'], axis=1)
df_clean.fillna(df_clean.mean(numeric_only=True), inplace=True)

categorical_cols = ['season', 'gender', 'region', 'primary_diagnosis',
                    'treatment_type', 'insurance_type', 'discharge_disposition']
df_final = pd.get_dummies(df_clean, columns=categorical_cols, drop_first=True)

X = df_final.drop('label', axis=1)
y = df_final['label']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=101)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

log_model = LogisticRegression(max_iter=1000)
log_model.fit(X_train, y_train)

y_pred = log_model.predict(X_test)
y_prob = log_model.predict_proba(X_test)[:, 1]

print("\n" + "=" * 30)
print("MODEL EVALUATION")
print("=" * 30)
print(f"ROC-AUC: {roc_auc_score(y_test, y_prob):.4f}")
print("Confusion Matrix:")
print(confusion_matrix(y_test, y_pred))
print("\nClassification Report:")
print(classification_report(y_test, y_pred))

# 6. SAVE ARTIFACTS for FastAPI backend
joblib.dump(log_model, "log_model.pkl")
joblib.dump(scaler, "scaler.pkl")
joblib.dump(X.columns, "columns.pkl")
print("Artifacts saved: log_model.pkl, scaler.pkl, columns.pkl")

# 7. INTERACTIVE RISK CHECKER (optional console use)
print("\n" + "="*30)
print("PATIENT READMISSION SYSTEM")
print("="*30)

while True:
    try:
        target_id = input("\nEnter Patient ID to check (e.g., P00001) or 'exit' to quit: ").strip()
    except EOFError:
        print("No interactive input available. Exiting System...")
        break

    if target_id.lower() == 'exit':
        print("Exiting System...")
        break

    patient_row = raw_data[raw_data['patient_id'] == target_id]

    if patient_row.empty:
        print(f"Error: ID {target_id} not found. Please try again.")
        continue

    patient_features = patient_row.drop(
        ['patient_id', 'admission_date', 'label', 'readmission_risk_score'],
        axis=1,
    )

    patient_encoded = pd.get_dummies(patient_features)
    patient_aligned = patient_encoded.reindex(columns=X.columns, fill_value=0)
    patient_scaled = scaler.transform(patient_aligned)

    risk_proba = log_model.predict_proba(patient_scaled)[0][1]
    risk_percent = risk_proba * 100

    print("-" * 30)
    print(f"RESULT FOR {target_id}:")
    print(f"Age: {patient_row['age'].values[0]}")
    print(f"Primary Diagnosis: {patient_row['primary_diagnosis'].values[0]}")
    print(f"Readmission Risk: {risk_percent:.2f}%")

    if risk_percent > 40:
        print("STATUS: HIGH RISK (Intervention Required)")
    else:
        print("STATUS: LOW RISK (Normal Follow-up)")
    print("-" * 30)
