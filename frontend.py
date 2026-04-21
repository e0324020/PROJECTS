import matplotlib.pyplot as plt
import pandas as pd
import requests
import seaborn as sns
import streamlit as st

st.set_page_config(page_title="Readmission Risk Dashboard", layout="wide")
st.title("Hospital Readmission Risk Dashboard")

BASE_URL = "http://127.0.0.1:8000"


def get_risk_level(risk):
    if risk >= 70:
        return "HIGH RISK"
    if risk >= 40:
        return "MODERATE RISK"
    return "LOW RISK"


def render_risk_visualization(predicted_risk, risk_level, historical_risk_score=None):
    st.subheader("Risk Visualization")

    metric_cols = st.columns(3)
    metric_cols[0].metric("Predicted Risk", f"{predicted_risk:.2f}%")
    metric_cols[1].metric("Risk Level", risk_level)
    metric_cols[2].metric(
        "Historical Score",
        f"{historical_risk_score:.2f}%" if historical_risk_score is not None else "N/A",
    )

    st.progress(min(max(predicted_risk / 100, 0.0), 1.0))

    chart_data = pd.DataFrame(
        {
            "Measure": ["Predicted Risk", "Alert Threshold"],
            "Value": [predicted_risk, 40.0],
        }
    )
    if historical_risk_score is not None:
        chart_data.loc[len(chart_data)] = ["Historical Score", historical_risk_score]

    fig, ax = plt.subplots(figsize=(8, 4))
    sns.barplot(data=chart_data, x="Measure", y="Value", palette="crest", ax=ax)
    ax.set_ylim(0, 100)
    ax.set_xlabel("")
    ax.set_ylabel("Risk (%)")
    ax.set_title("Patient Risk Snapshot")
    for container in ax.containers:
        ax.bar_label(container, fmt="%.1f%%")
    st.pyplot(fig)


def build_display_table(record):
    rows = []
    for key, value in record.items():
        if key in [
            "predicted_risk",
            "risk_level",
            "status",
            "historical_risk_score",
            "risk_visualization",
            "prediction_error",
        ]:
            continue

        if value is None:
            display_value = "N/A"
        elif isinstance(value, float):
            display_value = f"{value:.2f}"
        else:
            display_value = str(value)

        rows.append({"Field": key.replace("_", " ").title(), "Value": display_value})

    return pd.DataFrame(rows)


option = st.radio(
    "Choose an action:",
    ("Manual Entry for Prediction", "Check Patient by ID"),
)

# --- Manual Entry for Prediction ---
if option == "Manual Entry for Prediction":
    st.subheader("Enter Patient Data")

    col1, col2 = st.columns(2)
    with col1:
        patient_id = st.text_input("Patient ID (optional)")
        age = st.number_input("Age", min_value=0, max_value=120, value=55)
        gender = st.selectbox("Gender", ["Male", "Female"])
        season = st.selectbox("Season", ["Spring", "Summer", "Fall", "Winter"])
        region = st.selectbox("Region", ["South", "North", "East", "West"])
        primary_diagnosis = st.selectbox(
            "Primary Diagnosis",
            ["Diabetes", "Heart Disease", "COPD", "Hypertension", "Pneumonia", "Other"],
        )
        treatment_type = st.selectbox("Treatment Type", ["Medical", "Surgical", "Interventional"])

    with col2:
        insurance_type = st.selectbox("Insurance Type", ["Medicare", "Medicaid", "Private", "Uninsured"])
        discharge_disposition = st.selectbox(
            "Discharge Disposition",
            ["Home", "Home Health", "SNF", "Rehab", "Other"],
        )
        comorbidities_count = st.number_input("Comorbidities Count", min_value=0, max_value=20, value=2)
        length_of_stay = st.number_input("Length of Stay (days)", min_value=1, max_value=60, value=3)
        medications_count = st.number_input("Medications Count", min_value=0, max_value=30, value=5)
        followup_visits_last_year = st.number_input(
            "Follow-up Visits (last year)",
            min_value=0,
            max_value=20,
            value=2,
        )
        prev_readmissions = st.number_input("Previous Readmissions", min_value=0, max_value=10, value=0)

    if st.button("Predict Risk"):
        payload = {
            "patient_id": patient_id,
            "age": age,
            "gender": gender,
            "season": season,
            "region": region,
            "primary_diagnosis": primary_diagnosis,
            "comorbidities_count": comorbidities_count,
            "length_of_stay": length_of_stay,
            "treatment_type": treatment_type,
            "medications_count": medications_count,
            "followup_visits_last_year": followup_visits_last_year,
            "prev_readmissions": prev_readmissions,
            "insurance_type": insurance_type,
            "discharge_disposition": discharge_disposition,
        }
        try:
            response = requests.post(f"{BASE_URL}/predict", json=payload)
            if response.status_code == 200:
                result = response.json()
                if "error" in result:
                    st.error(result["error"])
                else:
                    risk = result["risk"]
                    risk_level = get_risk_level(risk)
                    if risk_level == "HIGH RISK":
                        st.error(f"Predicted Readmission Risk: {risk}% - {risk_level}")
                    elif risk_level == "MODERATE RISK":
                        st.warning(f"Predicted Readmission Risk: {risk}% - {risk_level}")
                    else:
                        st.success(f"Predicted Readmission Risk: {risk}% - {risk_level}")

                    render_risk_visualization(risk, risk_level)

                    resp_imp = requests.get(f"{BASE_URL}/feature_importance")
                    if resp_imp.status_code == 200:
                        feat_data = resp_imp.json()
                        if "error" not in feat_data:
                            feat_df = pd.DataFrame(feat_data)
                            fig, ax = plt.subplots(figsize=(10, 5))
                            sns.barplot(
                                x="Importance",
                                y="Feature",
                                data=feat_df,
                                palette="viridis",
                                ax=ax,
                            )
                            ax.set_title("Top 10 Factors Predicting Readmission")
                            st.pyplot(fig)
            else:
                st.error("Error: Could not get prediction")
        except Exception as e:
            st.error(f"Connection error: {e}")

# --- Check Patient by ID ---
elif option == "Check Patient by ID":
    st.subheader("Lookup Patient Record")

    patient_id = st.text_input("Enter Patient ID (e.g. P00001)")

    if st.button("Fetch Record"):
        if not patient_id.strip():
            st.warning("Please enter a Patient ID.")
        else:
            try:
                response = requests.get(f"{BASE_URL}/patient/{patient_id.strip()}")
                if response.status_code == 200:
                    record = response.json()

                    if "error" in record:
                        st.error(record["error"])
                    else:
                        risk = record.get("predicted_risk")
                        risk_level = record.get("risk_level") or record.get("status", "")
                        historical_risk_score = record.get("historical_risk_score")

                        if risk is not None:
                            if risk_level == "HIGH RISK":
                                st.error(f"Readmission Risk: {risk}% - {risk_level}")
                            elif risk_level == "MODERATE RISK":
                                st.warning(f"Readmission Risk: {risk}% - {risk_level}")
                            else:
                                st.success(f"Readmission Risk: {risk}% - {risk_level}")
                            render_risk_visualization(risk, risk_level, historical_risk_score)

                        st.subheader("Patient Details")
                        st.table(build_display_table(record))

                        resp_imp = requests.get(f"{BASE_URL}/feature_importance")
                        if resp_imp.status_code == 200:
                            feat_data = resp_imp.json()
                            if "error" not in feat_data:
                                feat_df = pd.DataFrame(feat_data)
                                fig, ax = plt.subplots(figsize=(10, 5))
                                sns.barplot(
                                    x="Importance",
                                    y="Feature",
                                    data=feat_df,
                                    palette="viridis",
                                    ax=ax,
                                )
                                ax.set_title("Top 10 Factors Predicting Readmission")
                                st.pyplot(fig)
                else:
                    st.error("Server error. Is the backend running?")
            except Exception as e:
                st.error(f"Connection error: {e}")
