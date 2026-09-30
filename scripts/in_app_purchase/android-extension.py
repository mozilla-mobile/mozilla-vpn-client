#!/usr/bin/env python3
"""Defer Google Play subscription expiry for a list of purchase tokens.

This is useful if we need to extend IAP subscriptions because of an outage or something similar.

Requires pyjwt
To install: `pip install pyjwt`

Service account JSON key with the Android Publisher scope is required. Subplat team
can give access via 1Password

Hits the Android Publisher REST API directly:
  GET  .../purchases/subscriptionsv2/tokens/{token}         -> read etag
  POST .../purchases/subscriptionsv2/tokens/{token}:defer   -> defer expiry
https://developers.google.com/android-publisher/api-ref/rest/v3/purchases.subscriptionsv2/defer
"""

import json
import sys
import time

import jwt
import requests

#### THESE LINES MUST BE UPDATED WITH ACCURATE PATHS. (I believe it needs to be full path, not relative.)
service_account_file = "/.../google-service-account.json" # see note above for access to this
subscriber_file = "/.../android-subscribers.txt"
####

DRY_RUN = True # Do not apply changes
DAYS_TO_EXTEND = 90


PACKAGE_NAME = "org.mozilla.firefox.vpn"
BASE_URL = "https://androidpublisher.googleapis.com/androidpublisher/v3"
TOKEN_URL = "https://oauth2.googleapis.com/token"
SCOPE = "https://www.googleapis.com/auth/androidpublisher"
TOKEN_LIFETIME_SECONDS = 30 * 60 # Google caps service-account assertions at 1 hour.
REQUEST_TIMEOUT = 30


def load_purchase_tokens(path):
    """One purchase token per line; blanks and #-comments ignored."""
    tokens = []
    with open(path, "r") as file:
        for line in file:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line in tokens:
                print(f"skipping duplicate: {line}")
                continue
            tokens.append(line)
    return tokens


class PlayClient:
    """Exchanges a service-account JWT for an access token and refreshes it."""

    def __init__(self, service_account):
        self.client_email = service_account["client_email"]
        self.private_key = service_account["private_key"]
        self.private_key_id = service_account.get("private_key_id")
        self.session = requests.Session()
        self._token = None
        self._token_expires_at = 0

    def access_token(self):
        now = int(time.time())
        # Refresh a minute early so a token can't expire mid-flight.
        if self._token and now < self._token_expires_at - 60:
            return self._token

        assertion = jwt.encode(
            {
                "iss": self.client_email,
                "scope": SCOPE,
                "aud": TOKEN_URL,
                "iat": now,
                "exp": now + TOKEN_LIFETIME_SECONDS,
            },
            self.private_key,
            algorithm="RS256",
            headers={"kid": self.private_key_id} if self.private_key_id else None,
        )

        response = self.session.post(
            TOKEN_URL,
            data={
                "grant_type": "urn:ietf:params:oauth:grant-type:jwt-bearer",
                "assertion": assertion,
            },
            timeout=REQUEST_TIMEOUT,
        )
        if response.status_code != 200:
            sys.exit(
                "Could not get an access token from Google:\n"
                f"  {describe_error(response)}\n"
                "Check that the service account key is valid and has not been revoked."
            )

        payload = response.json()
        self._token = payload["access_token"]
        self._token_expires_at = now + int(payload.get("expires_in", 3600))
        return self._token

    def request(self, method, path, json_body=None):
        headers = {
            "Authorization": f"Bearer {self.access_token()}",
            "Content-Type": "application/json",
        }
        return self.session.request(
            method,
            f"{BASE_URL}{path}",
            headers=headers,
            json=json_body,
            timeout=REQUEST_TIMEOUT,
        )

def subscription_path(purchase_token, suffix=""): # WAS (self, purchase_token, suffix=""):
    return (
        f"/applications/{PACKAGE_NAME}/purchases/subscriptionsv2/tokens/{purchase_token}{suffix}"
    )


def describe_error(response):
    """Google returns {"error": {"code", "message", "status"}} on failures."""
    try:
        body = response.json()
    except ValueError:
        return f"HTTP {response.status_code} {response.text[:200]}"

    error = body.get("error")
    if isinstance(error, dict):
        return (
            f"HTTP {response.status_code} "
            f"status={error.get('status')} {error.get('message')}"
        )
    if isinstance(error, str):
        # Token endpoint uses {"error": "...", "error_description": "..."}
        return f"HTTP {response.status_code} {error}: {body.get('error_description')}"
    return f"HTTP {response.status_code} {response.text[:200]}"


def defer_one(client, purchase_token):
    """Returns (ok, detail). Raises nothing.

    The API is compare-and-swap: the etag read here must still be current when
    the defer lands, otherwise Google rejects it.
    """
    try:
        current = client.request("GET", subscription_path(purchase_token))
    except requests.RequestException as e:
        return False, f"{type(e).__name__} on get: {e}"

    if current.status_code != 200:
        return False, f"get failed: {describe_error(current)}"

    try:
        subscription = current.json()
    except ValueError:
        return False, f"unparseable get response: {current.text[:200]}"

    etag = subscription.get("etag")
    if not etag:
        return False, "get response had no etag, cannot defer"

    state = subscription.get("subscriptionState")
    if state != "SUBSCRIPTION_STATE_ACTIVE":
        return False, f"not active (state={state}), skipping"

    body = {
        "deferralContext": {
            "etag": etag,
            "deferDuration": f"{DAYS_TO_EXTEND * 24*60*60}s",
            "validateOnly": DRY_RUN
        }
    }

    try:
        response = client.request(
            "POST", subscription_path(purchase_token, ":defer"), json_body=body
        )
    except requests.RequestException as e:
        return False, f"{type(e).__name__} on defer: {e}"

    if response.status_code != 200:
        if response.status_code in (409, 412):
            return False, f"etag conflict, subscription changed mid-run: {describe_error(response)}"
        return False, describe_error(response)

    try:
        payload = response.json()
    except ValueError:
        return False, f"unparseable defer response: {response.text[:200]}"

    details = payload.get("itemExpiryTimeDetails") or []
    if not details:
        return True, "deferred (no expiry details returned)"

    summary = ", ".join(
        f"{d.get('productId')} -> {d.get('expiryTime')}" for d in details
    )
    return True, summary


def main():
    purchase_tokens = load_purchase_tokens(subscriber_file)
    if not purchase_tokens:
        sys.exit("No purchase tokens found in the input file.")

    with open(service_account_file, "r") as f:
        service_account = json.load(f)

    client = PlayClient(service_account)

    print(
        f"[Dry run: {DRY_RUN}] {len(purchase_tokens)} subscriber(s), "
        f"+{DAYS_TO_EXTEND} days, package={PACKAGE_NAME}"
    )

    succeeded = 0
    failed = 0

    for i, purchase_token in enumerate(purchase_tokens, 1):
        ok, detail = defer_one(client, purchase_token)
        status = "ok" if ok else "error"
        succeeded += ok
        failed += not ok
        print(f"[{i}/{len(purchase_tokens)}] {purchase_token[:16]}...: {status} - {detail}")

    print(f"\nDone. {succeeded} ok, {failed} failed.")
    if DRY_RUN:
        print("Remember: This was a dry run. Nothing was applied")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
