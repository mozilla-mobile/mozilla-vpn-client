"""
This is useful if we need to extend IAP subscriptions because of an outage or something similar.

Requires https://github.com/apple/app-store-server-library-python

To install: `pip install app-store-server-library`

Private key with subscription permissions is required. The "individual API
key" you may be able to make in your own App Store Connect profile does not
have enough permissions. Heitor or other members of RelEng should be able to
give you access to the key you need.
"""

import os
import sys
import time
import uuid

from appstoreserverlibrary.api_client import AppStoreServerAPIClient, APIException
from appstoreserverlibrary.models.Environment import Environment
from appstoreserverlibrary.models.ExtendReasonCode import ExtendReasonCode
from appstoreserverlibrary.models.ExtendRenewalDateRequest import ExtendRenewalDateRequest

from appstoreserverlibrary.api_client import GetTransactionHistoryVersion
from appstoreserverlibrary.models.HistoryResponse import HistoryResponse
from appstoreserverlibrary.models.TransactionHistoryRequest import (
    TransactionHistoryRequest, ProductType, Order,
)

#### UPDATE THESE NEXT FIVE LINES
private_key_path = "/.../SubscriptionKey_2xxxxxxxxx.p8" # Update to full path
subscriber_file = "/.../extension-scripts/apple-subscribers.txt" # Update to full path
key_id = "2xxxxxxxxx" # probably the suffix of the key file
environment = Environment.SANDBOX # or Environment.PRODUCTION
issuer_id = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" # UUID that is available in App Store Connect. Go to Xcode cloud, pull from the URL.
####

bundle_id = "org.mozilla.ios.FirefoxVPN"
DAYS_TO_EXTEND = 90
EXTEND_REASON_CODE = ExtendReasonCode.CUSTOMER_SATISFACTION
SLEEP_BETWEEN_CALLS = 0.5 # to stay under rate limit

def load_transaction_ids(path):
    """One originalTransactionId per line; blanks and #-comments ignored."""
    ids = []
    with open(path, 'r') as file:
      for line in file:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line in ids:
            print(f"skipping duplicate: {line}")
            continue
        ids.append(line)
    return ids

def app_store_connect_client():
    if not os.path.isfile(private_key_path):
      sys.exit(f"Private key not found at {private_key_path}")
    with open(private_key_path, "rb") as binary_file:
        private_key = binary_file.read()

        client = AppStoreServerAPIClient(private_key, key_id, issuer_id, bundle_id, environment)
        try:    
            response = client.request_test_notification()
            print("we're working")
            print(response)
            return client
        except APIException as e:
            print("something in the setup is failing")
            print(e)
            exit(1)

def extend_one(client, transaction_id):
    """Returns (ok, detail). Raises nothing."""
    request = ExtendRenewalDateRequest(
        extendByDays=DAYS_TO_EXTEND,
        extendReasonCode=EXTEND_REASON_CODE,
        requestIdentifier=str(uuid.uuid4()),
    )
    try:
        response = client.extend_subscription_renewal_date(transaction_id, request)
    except APIException as e:
        return False, f"APIException status={e.http_status_code} code={e.api_error} {e.error_message}"
    except Exception as e:  # network, auth, etc.
        return False, f"{type(e).__name__}: {e}"

    if response.success:
        return True, f"new expiry {response.effectiveDate}"
    return False, f"success=false {response.originalTransactionId}"


def main():
    transaction_ids = load_transaction_ids(subscriber_file)
    if not transaction_ids:
        sys.exit("No transaction IDs found in the input file.")

    print(
        f"{len(transaction_ids)} subscriber(s), "
        f"+{DAYS_TO_EXTEND} days, reason={EXTEND_REASON_CODE.name}, env={environment}"
    )

    client = app_store_connect_client()

    # # client.trans
    # request = TransactionHistoryRequest(
    #     sort=Order.DESCENDING,
    #     revoked=False,
    #     productTypes=[ProductType.AUTO_RENEWABLE],
    # )
    # transactions = []
    # response: HistoryResponse = None
    # response = client.get_transaction_history(
    #         "410003469517580", None, request, GetTransactionHistoryVersion.V2
    # )
    # print(str(response))
    # exit(1)

    succeeded = 0
    failed = 0
    
    print(["original_transaction_id", "status", "detail"])

    for i, transaction_id in enumerate(transaction_ids, 1):
        ok, detail = extend_one(client, transaction_id)
        status = "ok" if ok else "error"
        succeeded += ok
        failed += not ok
        print(f"[{i}/{len(transaction_ids)}] {transaction_id}: {status} - {detail}")

        if i < len(transaction_ids):
            time.sleep(SLEEP_BETWEEN_CALLS)

    print(f"\nDone. {succeeded} extended, {failed} failed.")
    sys.exit(1 if failed else 0)

if __name__ == "__main__":
    main()
