/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@file:Suppress("KotlinJniMissingFunction")

package org.mozilla.firefox.vpn

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.util.Log
import com.android.billingclient.api.AcknowledgePurchaseParams
import com.android.billingclient.api.AcknowledgePurchaseResponseListener
import com.android.billingclient.api.BillingClient
import com.android.billingclient.api.BillingClientStateListener
import com.android.billingclient.api.BillingFlowParams
import com.android.billingclient.api.BillingResult
import com.android.billingclient.api.Purchase
import com.android.billingclient.api.PurchasesResponseListener
import com.android.billingclient.api.PurchasesUpdatedListener
import com.android.billingclient.api.SkuDetails
import com.android.billingclient.api.SkuDetailsParams
import com.android.billingclient.api.SkuDetailsResponseListener
import java.text.NumberFormat
import java.util.Currency
import kotlin.collections.ArrayList
import kotlin.collections.HashMap
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.Serializable

/**
 * Generally this contains the contents of a native BillingResult.
 * But in unexpected failure cases we use our own codes starting at -99.
 */
@Serializable
data class BillingResultData(
    val code: Int,
    val message: String
)

@Serializable
data class MozillaSubscriptionInfo(
    val id: String,
    val monthCount: Int
)

@Serializable
data class MozillaSubscriptions(
    val products: List<MozillaSubscriptionInfo>
)

@Serializable
data class GooglePlaySubscriptionInfo(
    val sku: String, // matches MozillaSubscriptionInfo.id
    val totalPriceString: String,
    val monthlyPriceString: String,
    val monthlyPrice: Double,
)

@Serializable
data class GooglePlaySubscriptions(
    val products: ArrayList<GooglePlaySubscriptionInfo>
)

class InAppPurchase private constructor(ctx: Context) :
    AcknowledgePurchaseResponseListener,
    BillingClientStateListener,
    PurchasesResponseListener,
    PurchasesUpdatedListener,
    SkuDetailsResponseListener {

    /**
     * SkuDetails, monthCounts, and Purchase by SKU
     */
    val skusWithSkuDetails = HashMap<String, SkuDetails>()
    val skusWithMonthCount = HashMap<String, Int>()
    val skusWithPurchase = HashMap<String, Purchase>()

    /**
     * The billingClient instance
     */
    private var billingClient = BillingClient.newBuilder(ctx)
        .setListener(this)
        .enablePendingPurchases() // Not used for subscriptions, but required.
        .build()

    /**
     * Functions in AndroidIAPHandler
     */
    // Success
    external fun onPurchaseAcknowledged()
    external fun onPurchaseUpdated(purchaseDataJSONBlob: String)
    external fun onSkuDetailsReceived(subscriptionsDataJSONBlob: String)
    // Failures
    external fun onBillingNotAvailable(billingResultJSONBlob: String)
    external fun onPurchaseAcknowledgeFailed(billingResultJSONBlob: String)
    external fun onSkuDetailsFailed(billingResultJSONBlob: String)
    external fun onSubscriptionFailed(billingResultJsonBlob: String)

    companion object {
        private const val TAG = "InAppPurchase"

        private var instance: InAppPurchase? = null

        @JvmStatic
        fun init(ctx: Context) {
            instance = instance ?: InAppPurchase(ctx)
        }

        @JvmStatic
        fun deinit() {
            instance?.billingClient?.endConnection()
        }

        @JvmStatic
        fun launchPlayStore(activity: Activity) {
            val intent = Intent.makeMainSelectorActivity(
                Intent.ACTION_MAIN,
                Intent.CATEGORY_APP_MARKET
            )
            activity.startActivity(intent)
        }

        @JvmStatic
        fun lookupProductsInPlayStore(productsToLookup: String) {
            instance?.initiateProductLookup(productsToLookup)
        }

        @JvmStatic
        fun purchaseProduct(productToPurchase: String, activity: Activity) {
            instance?.initiatePurchase(productToPurchase = productToPurchase, activity = activity)
        }

        @JvmStatic
        fun acknowledgePurchase(purchaseToken: String) {
            instance?.initiateAcknowledge(purchaseToken = purchaseToken)
        }
    }

    /**
     * Initiate functions
     */

    fun initiateProductLookup(productsToLookupRaw: String) {
        val productsToLookup = Json.decodeFromString<MozillaSubscriptions>(productsToLookupRaw)
        for (product in productsToLookup.products) {
            skusWithMonthCount[product.id] = product.monthCount
        }
        if (!billingClient.isReady) {
            Log.d(TAG, "BillingClient: Start connection...")
            billingClient.startConnection(this)
        } else {
            querySkuAndPurchases()
        }
    }

    fun initiatePurchase(productToPurchase: String, activity: Activity) {
        val skuDetails = skusWithSkuDetails[productToPurchase]
        if (skuDetails == null) {
            Log.wtf(TAG, "Attempting to purchase a product with no skuDetails")
            onSubscriptionFailed(
                Json.encodeToString(
                    BillingResultData(
                        code = -98,
                        message = "Attempted to purchase $productToPurchase with no sku details"
                    )
                )
            )
            return
        }
        val billingParams = BillingFlowParams
            .newBuilder()
            .setSkuDetails(skuDetails)
            // TODO - https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1537
            // .setObfuscatedAccountId(fxaId)
            .build()
        val billingResult = billingClient.launchBillingFlow(activity, billingParams)
        if (billingResult.responseCode != BillingClient.BillingResponseCode.OK) {
            onSubscriptionFailed(billingResultToJson(billingResult, "initiatePurchase"))
        }
    }

    fun initiateAcknowledge(purchaseToken: String) {
        val acknowledgePurchaseParams = AcknowledgePurchaseParams.newBuilder()
            .setPurchaseToken(purchaseToken)
            .build()
        billingClient.acknowledgePurchase(acknowledgePurchaseParams, this)
    }

    /**
     * Override functions for Billing Library listeners
     */

    override fun onBillingSetupFinished(billingResult: BillingResult) {
        if (billingResult.responseCode != BillingClient.BillingResponseCode.OK) {
            onBillingNotAvailable(billingResultToJson(billingResult, "onBillingSetupFinished"))
        } else {
            querySkuAndPurchases()
        }
    }

    override fun onBillingServiceDisconnected() {
        Log.i(TAG, "Billing Service Disconnected")
        skusWithSkuDetails.clear()
        skusWithMonthCount.clear()
        skusWithPurchase.clear()
        onBillingNotAvailable(
            Json.encodeToString(
                BillingResultData(
                    code = -99,
                    message = "Billing Service Disconnected"
                )
            )
        )
    }

    override fun onSkuDetailsResponse(
        billingResult: BillingResult,
        skuDetailsList: MutableList<SkuDetails>?
    ) {
        if (billingResult.responseCode != BillingClient.BillingResponseCode.OK) {
            onSkuDetailsFailed(billingResultToJson(billingResult, "onSkuDetailsResponse"))
            return
        }
        if (skuDetailsList == null) {
            onSkuDetailsFailed(
                Json.encodeToString(
                    BillingResultData(
                        code = -97,
                        message = "No sku details returned"
                    )
                )
            )
            return
        }
        val googleProducts = GooglePlaySubscriptions(products = arrayListOf())
        for (details in skuDetailsList) {
            val parsedDetails = skuDetailsToGooglePlaySubscriptionInfo(details)
            if (parsedDetails != null) {
                googleProducts.products.add(parsedDetails)
            }
        }
        val googleProductsJson = Json.encodeToString(googleProducts)
        Log.d(TAG, "Sending $googleProductsJson")
        onSkuDetailsReceived(googleProductsJson)
    }

    override fun onQueryPurchasesResponse(
        billingResult: BillingResult,
        purchases: MutableList<Purchase>
    ) {
        val responseCode = billingResult.responseCode
        if (responseCode == BillingClient.BillingResponseCode.OK) {
            processPurchases(purchases)
        } else {
            Log.e(TAG, "onQueryPurchasesReponse got BillingResponseCode $responseCode")
        }
    }

    override fun onPurchasesUpdated(
        billingResult: BillingResult,
        purchases: MutableList<Purchase>?
    ) {
        if (billingResult.responseCode == BillingClient.BillingResponseCode.OK) {
            processPurchases(purchases)
        } else {
            onSubscriptionFailed(billingResultToJson(billingResult, "onSkuDetailsResponse"))
        }
    }

    override fun onAcknowledgePurchaseResponse(billingResult: BillingResult) {
        if (billingResult.responseCode == BillingClient.BillingResponseCode.OK) {
            onPurchaseAcknowledged()
        } else {
            onPurchaseAcknowledgeFailed(
                billingResultToJson(billingResult, "onAcknowledgePurchaseResponse")
            )
        }
    }

    /**
     * The rest
     */

    fun querySkuAndPurchases() {
        if (!billingClient.isReady) {
            Log.d(TAG, "BillingClient: Start connection...")
            billingClient.startConnection(this)
        }
        val params = SkuDetailsParams.newBuilder()
            .setType(BillingClient.SkuType.SUBS)
            .setSkusList(skusWithMonthCount.keys.toList())
            .build()
        // Query skus
        params.let { skuDetailsParams ->
            Log.i(TAG, "querySkuDetailsAsync")
            billingClient.querySkuDetailsAsync(skuDetailsParams, this)
        }
        // Query existing subscription purchases
        billingClient.queryPurchasesAsync(BillingClient.SkuType.SUBS, this)
    }

    fun processPurchases(purchases: MutableList<Purchase>?) {
        if (purchases == null) {
            Log.d(TAG, "onPurchasesUpdated: null purchase list")
            return
        }
        for (purchase in purchases) {
            if (purchase.purchaseState != Purchase.PurchaseState.PURCHASED) {
                Log.i(TAG, "Purchase State is unexpectedly not PURCHASED")
            }
            onPurchaseUpdated(purchase.originalJson)
        }
    }

    fun skuDetailsToGooglePlaySubscriptionInfo(details: SkuDetails): GooglePlaySubscriptionInfo? {
        val sku = details.sku
        skusWithSkuDetails[sku] = details
        val priceMicros = details.priceAmountMicros
        val monthCount = skusWithMonthCount[sku]
        if (monthCount == null) {
            Log.e(TAG, "We did not get a monthCount for sku: $sku")
            return null
        }
        Log.d(TAG, "For sku $sku, we have $priceMicros priceMicros $monthCount months")
        val monthlyPrice = priceMicros / 1000000.00 / monthCount
        val formatter = NumberFormat.getCurrencyInstance()
        formatter.maximumFractionDigits = 2
        formatter.currency = Currency.getInstance(details.priceCurrencyCode)
        val monthlyPriceString = formatter.format(monthlyPrice)
        return GooglePlaySubscriptionInfo(
            totalPriceString = details.price,
            monthlyPriceString = monthlyPriceString,
            monthlyPrice = monthlyPrice,
            sku = sku
        )
    }

    fun billingResultToJson(billingResult: BillingResult, caller: String): String {
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        Log.d(TAG, "BillingResult from $caller: $responseCode $debugMessage")
        return Json.encodeToString(
            BillingResultData(
                code = responseCode,
                message = debugMessage
            )
        )
    }
}
