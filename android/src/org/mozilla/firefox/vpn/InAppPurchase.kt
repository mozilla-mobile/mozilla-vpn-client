/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@file:Suppress("KotlinJniMissingFunction")

package org.mozilla.firefox.vpn

import android.app.Activity
import android.content.Context
import android.util.Log
import com.android.billingclient.api.BillingClient
import com.android.billingclient.api.BillingClientStateListener
import com.android.billingclient.api.BillingFlowParams
import com.android.billingclient.api.BillingResult
import com.android.billingclient.api.Purchase
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
    BillingClientStateListener, SkuDetailsResponseListener, PurchasesUpdatedListener {

    /**
     * SkuDetails and monthCounts by SKU
     */
    val skusWithSkuDetails = HashMap<String, SkuDetails>()
    val skusWithMonthCount = HashMap<String, Int>()

    /**
     * The billingClient instance
     */
    private var billingClient = BillingClient.newBuilder(ctx)
        .setListener(this)
        .enablePendingPurchases() // Not used for subscriptions.
        .build()

    // Functions in AndroidIAPHandler
    external fun onSkuDetailsReceived(subscriptionsDataJSONBlob: String)
    external fun onNoPurchases()
    external fun onPurchaseCanceled()
    external fun onPurchaseUpdated(purchaseDataJSONBlob: String)

    companion object {
        private const val TAG = "InAppPurchase"

        private var instance: InAppPurchase? = null

        /**
         * TODO - Discuss / get consult from basti
         * Because we launch IAPHandler in mozillavpn, I believe all users will be forced
         * to have Play and be logged in irrespective of whether they need to IAP.
         * But the dependency on billing library may have this effect anyway.
         */
        @JvmStatic
        fun init(ctx: Context) {
            instance = instance ?: InAppPurchase(ctx)
        }

        @JvmStatic
        fun deinit() {
            instance?.billingClient?.endConnection()
        }

        @JvmStatic
        fun lookupProductsInPlayStore(productsToLookup: String) {
            instance?.initiateProductLookup(productsToLookup)
        }

        @JvmStatic
        fun purchaseProduct(productToPurchase: String, activity: Activity) {
            instance?.initiatePurchase(productToPurchase = productToPurchase, activity = activity)
        }
    }

    fun initiateProductLookup(productsToLookupRaw: String) {
        val productsToLookup = Json.decodeFromString<MozillaSubscriptions>(productsToLookupRaw)
        for (product in productsToLookup.products) {
            skusWithMonthCount[product.id] = product.monthCount
        }
        if (!billingClient.isReady) {
            Log.d(TAG, "BillingClient: Start connection...")
            billingClient.startConnection(this)
        } else {
            querySkuDetails()
        }
    }

    override fun onBillingSetupFinished(billingResult: BillingResult) {
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        if (responseCode != BillingClient.BillingResponseCode.OK) {
            Log.e(TAG, "onBillingSetupFinished was not successful: $responseCode $debugMessage")
        } else {
            querySkuDetails()
        }
    }

    override fun onBillingServiceDisconnected() {
        Log.i(TAG, "SkuDetails Billing Service Disconnected")
        // TODO - Call back to clean-up Qt side?
    }

    fun querySkuDetails() {
        if (!billingClient.isReady) {
            Log.d(TAG, "BillingClient: Start connection...")
            billingClient.startConnection(this)
        }
        val params = SkuDetailsParams.newBuilder()
            .setType(BillingClient.SkuType.SUBS)
            .setSkusList(skusWithMonthCount.keys.toList())
            .build()
        params.let { skuDetailsParams ->
            Log.i(TAG, "querySkuDetailsAsync")
            billingClient.querySkuDetailsAsync(skuDetailsParams, this)
        }
    }

    override fun onSkuDetailsResponse(
        billingResult: BillingResult,
        skuDetailsList: MutableList<SkuDetails>?
    ) {
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        when (responseCode) {
            BillingClient.BillingResponseCode.OK -> {
                Log.d(TAG, "onSkuDetailsResponse: $responseCode $debugMessage")
                if (skuDetailsList == null) {
                    Log.e(TAG, "Found null SkuDetails.")
                } else {
                    val googleProducts = GooglePlaySubscriptions(products = arrayListOf())
                    for (details in skuDetailsList) {
                        val sku = details.sku
                        skusWithSkuDetails[sku] = details
                        val priceMicros = details.priceAmountMicros
                        val monthCount = skusWithMonthCount[sku]
                        if (monthCount == null) {
                            Log.e(TAG, "We did not get a monthCount for sku: $sku")
                            return
                        }
                        Log.d(
                            TAG,
                            "For sku $sku, we have $priceMicros priceMicros $monthCount months"
                        )
                        val monthlyPrice = priceMicros / 1000000.00 / monthCount
                        val formatter = NumberFormat.getCurrencyInstance()
                        formatter.maximumFractionDigits = 2
                        formatter.currency = Currency.getInstance(details.priceCurrencyCode)
                        val monthlyPriceString = formatter.format(monthlyPrice)
                        googleProducts.products.add(
                            GooglePlaySubscriptionInfo(
                                totalPriceString = details.price,
                                monthlyPriceString = monthlyPriceString,
                                monthlyPrice = monthlyPrice,
                                sku = sku
                            )
                        )
                    }
                    val googleProductsJson = Json.encodeToString(googleProducts)
                    Log.d(TAG, "Sending $googleProductsJson")
                    onSkuDetailsReceived(googleProductsJson)
                }
            }
            BillingClient.BillingResponseCode.SERVICE_DISCONNECTED,
            BillingClient.BillingResponseCode.SERVICE_UNAVAILABLE,
            BillingClient.BillingResponseCode.BILLING_UNAVAILABLE,
            BillingClient.BillingResponseCode.ITEM_UNAVAILABLE,
            BillingClient.BillingResponseCode.DEVELOPER_ERROR,
            BillingClient.BillingResponseCode.ERROR -> {
                Log.e(TAG, "onSkuDetailsResponse: $responseCode $debugMessage")
            }
            BillingClient.BillingResponseCode.USER_CANCELED,
            BillingClient.BillingResponseCode.FEATURE_NOT_SUPPORTED,
            BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED,
            BillingClient.BillingResponseCode.ITEM_NOT_OWNED -> {
                // These response codes are not expected.
                Log.wtf(TAG, "onSkuDetailsResponse: $responseCode $debugMessage")
            }
        }
    }

    fun initiatePurchase(productToPurchase: String, activity: Activity) {
        val skuDetails = skusWithSkuDetails[productToPurchase]
        if (skuDetails == null) {
            Log.wtf(TAG, "Attempting to purchase a product with no skuDetails")
            // TODO - What do we want to do in this case?
            return
        }
        val billingParams = BillingFlowParams
            .newBuilder()
            .setSkuDetails(skuDetails)
            .build()
        val billingResult = billingClient.launchBillingFlow(activity, billingParams)
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        Log.d(TAG, "launchBillingFlow: BillingResponse $responseCode $debugMessage")
    }

    /**
     * Called by the Billing Library when new purchases are detected.
     */
    override fun onPurchasesUpdated(
        billingResult: BillingResult,
        purchases: MutableList<Purchase>?
    ) {
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        Log.d(TAG, "onPurchasesUpdated: $responseCode $debugMessage")
        when (responseCode) {
            BillingClient.BillingResponseCode.OK -> {
                if (purchases == null) {
                    Log.d(TAG, "onPurchasesUpdated: null purchase list")
                    onNoPurchases()
                } else {
                    for (purchase in purchases) {
                        Log.d(TAG, "onPurchasesUpdated $purchase.originalJson")
                        onPurchaseUpdated(purchase.originalJson)
                    }
                }
            }
            BillingClient.BillingResponseCode.USER_CANCELED -> {
                // This happens when user hits "back" button from native
                // purchase screen.
                Log.i(TAG, "onPurchasesUpdated: User canceled the purchase")
                onPurchaseCanceled()
            }
            // TODO - How do we want to handle these cases
            BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED -> {
                Log.i(TAG, "onPurchasesUpdated: The user already owns this item")
            }
            BillingClient.BillingResponseCode.DEVELOPER_ERROR -> {
                Log.e(TAG, "onPurchasesUpdated: Developer error")
            }
        }
    }
}
