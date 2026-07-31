/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

@file:Suppress("KotlinJniMissingFunction")

package org.mozilla.firefox.vpn.qt

import android.app.Activity
import android.content.Context
import android.util.Log
import com.android.billingclient.api.AcknowledgePurchaseParams
import com.android.billingclient.api.AcknowledgePurchaseResponseListener
import com.android.billingclient.api.BillingClient
import com.android.billingclient.api.BillingClientStateListener
import com.android.billingclient.api.BillingFlowParams
import com.android.billingclient.api.BillingResult
import com.android.billingclient.api.QueryProductDetailsParams
import com.android.billingclient.api.QueryProductDetailsResult
import com.android.billingclient.api.QueryPurchasesParams
import com.android.billingclient.api.PendingPurchasesParams
import com.android.billingclient.api.ProductDetails
import com.android.billingclient.api.ProductDetailsResponseListener
import com.android.billingclient.api.Purchase
import com.android.billingclient.api.PurchasesResponseListener
import com.android.billingclient.api.PurchasesUpdatedListener
import kotlinx.serialization.Serializable
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import java.text.NumberFormat
import java.time.Duration
import java.time.format.DateTimeParseException
import java.util.Currency
import kotlin.collections.ArrayList
import kotlin.collections.HashMap

/**
 * Generally this contains the contents of a native BillingResult.
 * But in unexpected failure cases we use our own codes starting at -99.
 */
@Serializable
data class BillingResultData(
    val code: Int,
    val message: String,
)

@Serializable
data class MozillaSubscriptionInfo(
    val id: String,
    val monthCount: Int,
)

@Serializable
data class MozillaSubscriptions(
    val products: List<MozillaSubscriptionInfo>,
)

@Serializable
data class GooglePlaySubscriptionInfo(
    val sku: String, // matches MozillaSubscriptionInfo.id
    val totalPriceString: String,
    val monthlyPriceString: String,
    val monthlyPrice: Double,
    val trialDays: Int,
    val currencyCode: String,
)

@Serializable
data class GooglePlaySubscriptions(
    val products: ArrayList<GooglePlaySubscriptionInfo>,
)

class InAppPurchase private constructor(ctx: Context) :
    AcknowledgePurchaseResponseListener,
    BillingClientStateListener,
    PurchasesResponseListener,
    PurchasesUpdatedListener,
    ProductDetailsResponseListener {

    /**
     * ProductDetails, monthCounts, and Purchase by SKU
     */
    val idsWithProductDetails = HashMap<String, ProductDetails>()
    val idsWithMonthCount = HashMap<String, Int>()

    /**
     * The billingClient instance
     */

    val pendingPurchasesParams = PendingPurchasesParams.newBuilder()
        .enableOneTimeProducts()
        .build()
    private var billingClient = BillingClient.newBuilder(ctx)
        .setListener(this)
        .enablePendingPurchases(pendingPurchasesParams)
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
            idsWithMonthCount[product.id] = product.monthCount
        }
        if (!billingClient.isReady) {
            Log.d(TAG, "BillingClient: Start connection...")
            billingClient.startConnection(this)
        } else {
            querySkuAndPurchases()
        }
    }

    fun initiatePurchase(productToPurchase: String, activity: Activity) { // PRODUCTTOPURCHASE MUST BE PRODUCTID
        val productDetails = idsWithProductDetails[productToPurchase]
        if (productDetails == null) {
            Log.wtf(TAG, "Attempting to purchase a product with no productDetails")
            onSubscriptionFailed(
                Json.encodeToString(
                    BillingResultData(
                        code = -98,
                        message = "Attempted to purchase $productToPurchase with no sku details",
                    ),
                ),
            )
            return
        }
        val offerToken = productDetails.subscriptionOfferDetails
            ?.firstOrNull()
            ?.offerToken
            ?: run {
                onSubscriptionFailed(
                  Json.encodeToString(
                    BillingResultData(
                        code = -96,
                        message = "No offer found",
                    ),
                ),
                )
                return
            }

        val billingParams = BillingFlowParams.newBuilder()
            .setProductDetailsParamsList(
                listOf(
                    BillingFlowParams.ProductDetailsParams.newBuilder()
                        .setProductDetails(productDetails)
                        .setOfferToken(offerToken)
                        .build()
                )
            )
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
        idsWithProductDetails.clear()
        idsWithMonthCount.clear()
        onBillingNotAvailable(
            Json.encodeToString(
                BillingResultData(
                    code = -99,
                    message = "Billing Service Disconnected",
                ),
            ),
        )
    }

    override fun onProductDetailsResponse(
        billingResult: BillingResult,
        result: QueryProductDetailsResult,
    ) {
        if (billingResult.responseCode != BillingClient.BillingResponseCode.OK) {
            onSkuDetailsFailed(billingResultToJson(billingResult, "onProductDetailsResponse"))
            return
        }
        val productDetailsList = result.productDetailsList
        val googleProducts = GooglePlaySubscriptions(products = arrayListOf())
        for (details in productDetailsList) {
            val parsedDetails = productDetailsToGooglePlaySubscriptionInfo(details)
            Log.d(TAG, "Sending $parsedDetails")
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
        purchases: MutableList<Purchase>,
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
        purchases: MutableList<Purchase>?,
    ) {
        if (billingResult.responseCode == BillingClient.BillingResponseCode.OK) {
            processPurchases(purchases)
        } else {
            val reason = when (billingResult.responseCode) {
                BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED -> "item-already-owned"
                BillingClient.BillingResponseCode.USER_CANCELED -> "user-canceled"
                BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED -> "item-unavailable"
                else -> "unknown-${billingResult.responseCode}"
            }
            onSubscriptionFailed(billingResultToJson(billingResult, "onSkuDetailsResponse"))
        }
    }

    override fun onAcknowledgePurchaseResponse(billingResult: BillingResult) {
        if (billingResult.responseCode == BillingClient.BillingResponseCode.OK) {
            onPurchaseAcknowledged()
        } else {
            onPurchaseAcknowledgeFailed(
                billingResultToJson(billingResult, "onAcknowledgePurchaseResponse"),
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
        val params = QueryProductDetailsParams.newBuilder()
            .setProductList(
                idsWithMonthCount.keys.map { productId ->
                    QueryProductDetailsParams.Product.newBuilder()
                        .setProductId(productId)
                        .setProductType(BillingClient.ProductType.SUBS)
                        .build()
                }
            )
            .build()

        // Query skus
        params.let { productDetailsParams ->
            Log.i(TAG, "querySkuDetailsAsync")
            billingClient.queryProductDetailsAsync(productDetailsParams, this)
        }
        // Query existing subscription purchases
        val queryParams = QueryPurchasesParams.newBuilder()
            .setProductType(BillingClient.ProductType.SUBS)
            .build()
        billingClient.queryPurchasesAsync(queryParams, this)
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

    fun productDetailsToGooglePlaySubscriptionInfo(details: ProductDetails): GooglePlaySubscriptionInfo? {
        val productId = details.productId
        idsWithProductDetails[productId] = details

        // freeTrialPeriod is a ISO 8601 duration i.e P7D == 7 days
        val offers = details.subscriptionOfferDetails ?: return null
        val allPhases = offers.flatMap { it.pricingPhases.pricingPhaseList }
        val trialPhase = allPhases.firstOrNull { it.priceAmountMicros == 0L }
        val trialDays = if ((trialPhase?.billingPeriod ?: "").isEmpty()) {
            0
        } else {
            try {
                val duration = Duration.parse(trialPhase?.billingPeriod)
                duration.toDays().toInt()
            } catch (e: DateTimeParseException) {
                if (trialPhase?.billingPeriod == "P1W") {
                    // Google Play store and Java seem to disagree on ISO 8601
                    // Google will use 1W for 7 days, java.Duration however forbids using W
                    7
                } else {
                    Log.e(TAG, "Failed to Parse Trial duration ${trialPhase?.billingPeriod}")
                    0
                }
            }
        }

        val monthCount = idsWithMonthCount[productId]
        if (monthCount == null) {
            Log.e(TAG, "We did not get a monthCount for productId: $productId")
            return null
        }
        val basePhase = allPhases.lastOrNull { it.priceAmountMicros != 0L } ?: return null
        Log.d(TAG, "For productId $productId, we have $basePhase.priceAmountMicros priceMicros $monthCount months")
        val monthlyPrice = basePhase.priceAmountMicros / 1000000.00 / monthCount

        val formatter = NumberFormat.getCurrencyInstance()
        formatter.maximumFractionDigits = 2
        formatter.currency = Currency.getInstance(basePhase.priceCurrencyCode)
        val monthlyPriceString = formatter.format(monthlyPrice)

        return GooglePlaySubscriptionInfo(
            currencyCode = basePhase.priceCurrencyCode,
            totalPriceString = basePhase.formattedPrice,
            trialDays = trialDays,
            monthlyPriceString = monthlyPriceString,
            monthlyPrice = monthlyPrice,
            sku = productId,
        )
    }

    fun billingResultToJson(billingResult: BillingResult, caller: String): String {
        val responseCode = billingResult.responseCode
        val debugMessage = billingResult.debugMessage
        Log.d(TAG, "BillingResult from $caller: $responseCode $debugMessage")
        return Json.encodeToString(
            BillingResultData(
                code = responseCode,
                message = debugMessage,
            ),
        )
    }
}
