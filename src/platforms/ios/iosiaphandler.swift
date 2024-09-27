import Foundation
import StoreKit

@available(iOS 15, *)
@objc class InAppPurchaseHandler: NSObject {
  private static let logger = IOSLoggerImpl(tag: "InAppPurchaseHandler")

  // Saving the product list in this class is awful, but we're in a tough spot here.
  // The older StoreKit API used both ObjC and Swift, and we could save the ObjC object
  // as part of the ProductsHandler class, which is written in C++.
  // The new StoreKit API is Swift-only. We'd have to write a bunch of wrapper code to save
  // the object in C++, marshalling the object into and out of Swift. Given how brittle that is,
  // we instead having a shadow product cache in this Swift class, which we use if the user wants
  // to subscribe.
  private var productList: [Product] = []

  var transactionUpdates: Task<Void, Never>? = nil
  var errorCallback: (() -> Void)
  var successCallback: ((NSString, NSString) -> Void)

  @objc init(errorCallback: @escaping (() -> Void), successCallback: @escaping ((NSString, NSString) -> Void)) {
    self.errorCallback = errorCallback
    self.successCallback = successCallback
    super.init()
    transactionUpdates = newTransactionListenerTask()
  }

  deinit {
    transactionUpdates?.cancel()
  }

  // MARK: - Getting products
  @objc func getProducts(with productIdentifiers: Set<String>, productRegistrationCallback: @escaping ((NSString, NSString, NSString, NSString, Double, Int) -> Void), registrationCompleteCallback: @escaping (() ->Void), registrationFailureCallback: @escaping (() ->Void)) async {
      do {
        productList = try await Product.products(for: Array(productIdentifiers))
        InAppPurchaseHandler.logger.info(message: "Saved \(productList.count) products")

        for product in productList {

          let productIdentifier = product.id
          InAppPurchaseHandler.logger.debug (message: "\(productIdentifier): \(product.displayName)")
          InAppPurchaseHandler.logger.debug (message: product.description)

          guard let subscription = product.subscription else {
            InAppPurchaseHandler.logger.error(message: "Product does not include subscription: \(product.id)")
            assertionFailure()
            continue
          }

          let trialDays: Int
          if await subscription.isEligibleForIntroOffer, let offerType = subscription.introductoryOffer?.type, offerType == .introductory, let introOfferPeriod = subscription.introductoryOffer?.period {
            trialDays = daysInTrial(for: introOfferPeriod.unit, with: introOfferPeriod.value)
          } else {
            trialDays = 0
          }

          let price = product.displayPrice
          let currencyCode = product.priceFormatStyle.currencyCode

          let numberOfMonthsInSubscription = monthsInSubscription(for: subscription.subscriptionPeriod.unit, with: subscription.subscriptionPeriod.value)
          let monthlyPrice: Double = (product.price as NSDecimalNumber).doubleValue / Double(numberOfMonthsInSubscription)

          let numberFormatter = NumberFormatter()
          numberFormatter.numberStyle = .currency
          numberFormatter.locale = product.priceFormatStyle.locale
          guard let monthlyPriceString = numberFormatter.string(from: NSNumber(floatLiteral: monthlyPrice)) else {
            InAppPurchaseHandler.logger.error(message: "Monthly price could not be converted to a string with numberFormatter: \(monthlyPrice)")
            assertionFailure()
            continue
          }

          productRegistrationCallback(NSString(string: productIdentifier), NSString(string: currencyCode), NSString(string: price), NSString(string: monthlyPriceString), monthlyPrice, trialDays)
        }

        registrationCompleteCallback()

        } catch {
          InAppPurchaseHandler.logger.error(message: "Failed product request from the App Store server. \(error)")
          registrationFailureCallback()
          assertionFailure()
        }
    }

  // MARK: - Starting subscription
  @objc func startSubscription(for productIdentifier: String) async {
    guard let product = productList.first(where: {$0.id == productIdentifier}) else {
      InAppPurchaseHandler.logger.error(message: "Could not find a product with ID \(productIdentifier)")
      errorCallback()
      assertionFailure()
      return
    }

    do {
      let purchaseResult = try await product.purchase()
      await handlePurchaseResult(purchaseResult)
    } catch {
      InAppPurchaseHandler.logger.error(message: "Error purchasing product: \(error.localizedDescription)")
      errorCallback()
      assertionFailure()
    }
  }

  private func handlePurchaseResult(_ purchaseResult: Product.PurchaseResult) async {
    switch purchaseResult {
    case .success(let verificationResult):
      switch verificationResult {
      case .verified(let transaction):
        InAppPurchaseHandler.logger.info(message: "StoreKit subscription returned success and verified")
        let originalTransactionIdentifier: String = "\(transaction.originalID)"
        successCallback(NSString(string: transaction.productID), NSString(string: originalTransactionIdentifier))
        await transaction.finish()
      case .unverified(_, let verificationError):
        InAppPurchaseHandler.logger.info(message: "StoreKit subscription returned success, but unverified: \(verificationError.localizedDescription)")
        errorCallback()
      }
    case .pending:
      InAppPurchaseHandler.logger.info(message: "StoreKit subscription returned pending")
      // do not call error nor success - wait for further updates via `Transaction.updates`
    case .userCancelled:
      InAppPurchaseHandler.logger.info(message: "StoreKit subscription returned userCancelled")
      errorCallback()
    @unknown default:
      InAppPurchaseHandler.logger.error(message: "purchaseResult using a new enum")
      errorCallback()
    }
  }

  @objc func restoreSubscriptions() async {
    InAppPurchaseHandler.logger.info(message: "Syncing subscriptions via StoreKit2")
    try? await AppStore.sync()
    var didFindVerifiedTransaction = false
    for await verificationResult in Transaction.currentEntitlements {
        switch verificationResult {
        case .verified(let transaction):
          InAppPurchaseHandler.logger.info(message: "Found verified transaction")
          didFindVerifiedTransaction = true
          let originalTransactionIdentifier: String = "\(transaction.originalID)"
          successCallback(NSString(string: transaction.productID), NSString(string: originalTransactionIdentifier))
        case .unverified(let unverifiedTransaction, let verificationError):
          InAppPurchaseHandler.logger.info(message: "Found unverified transaction \(unverifiedTransaction.originalID) with error: \(verificationError.localizedDescription)")
        }
    }
    if !didFindVerifiedTransaction {
      InAppPurchaseHandler.logger.info(message: "Found no verified transactions")
      errorCallback()
    }
  }

  private func monthsInSubscription(for unit: Product.SubscriptionPeriod.Unit, with length: Int) -> Int {
    switch unit {
    case .day: return length / 30 // using 30 days per month - should never get this subscription unit
    case .week: return length / 4 // using 4 weeks per month - should never get this subscription unit
    case .month: return length // this one is easy!
    case .year: return length * 12 // 12 months per year
    @unknown default:
      InAppPurchaseHandler.logger.error(message: "unit using a new enum")
      return length
    }
  }

  private func daysInTrial(for unit: Product.SubscriptionPeriod.Unit, with length: Int) -> Int {
    switch unit {
    case .day: return length
    case .week: return length * 7 // 7 days per week
    case .month: return length * 30 // using 30 days per month
    case .year: return length * 365 // sorry leap years!
    @unknown default:
      InAppPurchaseHandler.logger.error(message: "unit using a new enum")
      return length
    }
  }

  // MARK: - Listening for new transactions
  private func newTransactionListenerTask() -> Task<Void, Never> {
    InAppPurchaseHandler.logger.info(message: "Creating transaction listener")
      return Task(priority: .background) {
          for await verificationResult in Transaction.updates {
              self.handle(updatedTransaction: verificationResult)
          }
      }
  }

  private func handle(updatedTransaction verificationResult: VerificationResult<Transaction>) {
    guard case .verified(let transaction) = verificationResult else {
        InAppPurchaseHandler.logger.info(message: "Unverified transaction returend")
        return
    }

    if let _ = transaction.revocationDate {
      InAppPurchaseHandler.logger.info(message: "Transaction was revoked")
      errorCallback()
    } else if let expirationDate = transaction.expirationDate,
        expirationDate < Date() {
      // Expirations handled by server
      InAppPurchaseHandler.logger.info(message: "Transaction has expired")
      errorCallback()
        return
    } else if transaction.isUpgraded {
        // Do nothing, there is an active transaction
        // for a higher level of service.
      InAppPurchaseHandler.logger.info(message: "Transaction was upgraded")
      errorCallback()
        return
    } else {
      InAppPurchaseHandler.logger.info(message: "New successful transaction returned")
      let originalTransactionIdentifier: String = "\(transaction.originalID)"
      successCallback(NSString(string: transaction.productID), NSString(string: originalTransactionIdentifier))
    }
  }
}
