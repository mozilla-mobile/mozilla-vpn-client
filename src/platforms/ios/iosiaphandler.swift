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

  @objc func startSubscription(for productIdentifier: String, errorCallback: @escaping (() -> Void), successCallback: @escaping ((NSString) -> Void)) async {
    guard let product = productList.first(where: {$0.id == productIdentifier}) else {
      InAppPurchaseHandler.logger.error(message: "Could not find a product with ID \(productIdentifier)")
      errorCallback()
      assertionFailure()
      return
    }

    do {
      let purchaseResult = try await product.purchase()
      switch purchaseResult {
      case .success(let verificationResult):
        switch verificationResult {
        case .verified(let transaction):
          InAppPurchaseHandler.logger.error(message: "StoreKit subscription returned success and verified")
          let originalTransactionIdentifier: String = "\(transaction.originalID)"
          successCallback(NSString(string: originalTransactionIdentifier))
          await transaction.finish()
        case .unverified(_, let verificationError):
          InAppPurchaseHandler.logger.error(message: "StoreKit subscription returned success, but unverified: \(verificationError.localizedDescription)")
          errorCallback()
          break
        }
      case .pending:
        InAppPurchaseHandler.logger.error(message: "StoreKit subscription returned pending")
        // do not call error nor success - wait for further updates via `Transaction.updates`
        break
      case .userCancelled:
        InAppPurchaseHandler.logger.error(message: "StoreKit subscription returned userCancelled")
        errorCallback()
        break
      @unknown default:
        InAppPurchaseHandler.logger.error(message: "purchaseResult using a new enum")
        errorCallback()
        break
      }
    } catch {
      InAppPurchaseHandler.logger.error(message: "Error purchasing product: \(error.localizedDescription)")
      errorCallback()
      assertionFailure()
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
}
