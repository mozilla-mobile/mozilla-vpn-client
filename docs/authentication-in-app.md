# VPN Authentication in-app

This page describes how the implementation for the authentication in-app works
in the Mozilla VPN client.

To know more about the FxA authentication flow, see the official [FxA API
documentation](https://github.com/mozilla/fxa/blob/main/packages/fxa-auth-server/docs/api.md)

## Authentication methods

The authentication is triggered by the frontend component, calling the method
`VPN.authenticate()`. The client picks the right authentication method based on
what is supported by the current platform. There are mainly 3 methods. The
current configuration for 2.8 is the following:
- Android: in-app authentication with account creation
- iOS: in-app authentication with account creation
- Linux: web-based authentication
- Windows: in-app authentication without account creation
- MacOS: in-app authentication without account creation.

### In-app authentication with account creation

The client allows sign-in and sign-up.

### In-app authentication without account creation

The client allows the sign-in. But if the account doesn’t exist, instead of
starting the account creation in-app, the flow continues in the browser as we
used to do in previous versions.

### Web-based authentication

This flow is what we currently use in v2.7 and previous versions.

## Finite State machine

The authentication flow can be seen and implemented as a [finite state
machine](https://en.wikipedia.org/wiki/Finite-state_machine).
The states are described in the next sections, For each state, we have a list
of error codes to describe what goes wrong. At the end of this document, you
can read the list of all the error codes. On top of these errors, we have the
main ones such as “No internet connection”.

Almost all the states have a call to action (write the email address or the
password, or the 6-digit codes, etc). After this, the frontend code must call
one or more available methods (see the list per states) to move to the next
state or to trigger an error.

### State: Initialization

This is an internal state in which the VPN client communicates with the
guardian-website to start the process. If there are no connectivity issues,
this state takes almost nothing to be completed.
From here, we go to **Start**.

* Next states: **Start**
* Errors: none
* Available methods: none

### State: Start

This state is the starting point for the in-app authentication flow. In this
state, the client asks for the email address. The tasks for this state are:

- Obtain the email address
- Validate the email address
- Process the email address

When the email address is received and validated, the client checks if there is
an account associated with this address. This operation can take a few seconds
(usually less).

If the account exists, we go to **Sign-in**, otherwise, for a new account, we
go to **Sign-up** or **fallback in the browser**, in case the authentication
method is “without account creation”.

* Next states: **Sign-in**, **Sign-up**, **fallback in the browser**
* Errors: none
* Available methods:
    * `VPNAuthInApp.checkAccount(emailAddress)`
    * `VPNAuthInApp.validateEmailAddress(emailAddress) -> bool`

### State: Sign-in

The client asks for the password. Then, it interacts with FxA. This should take
a few seconds (usually less). From here we can go to several states:
- **Unblock code needed**: FxA requires an extra validation step. An email is
  sent to the user with a 6-digit code.
- **Totp verification needed**: the account is configured to use TOTP (double
  factor authentication). The user needs to submit a 6-digit code.
- **Finalize**: the authentication is ready to be finalized

* Next states: **Unblock code needed**, **Totp verification needed**,
  **Finalize**
* Errors:
    * Unknown account - the account does not exist anymore. Super rare.
    * Incorrect password
    * Sign in with this email type is not currently supported -This is a
      strange FxA code. We have to investigate how to reproduce it.
    * Failed to send email - the unblock code is needed but the email sending
      failed.
* Available methods:
    * `VPNAuthInApp.setPassword(password)`
    * `VPNAuthInApp.signIn()`

### State: unblock code needed

This state is shown if FxA “doesn’t trust” the account or the user enough. An
email is sent to the user’s email address with a 6-digit code. In this state,
the client needs to inform the user and ask for this 6-digit code.

If needed, the user can ask to have a new email code. See:
`VPNAuthInApp.resendUnblockCodeEmail()`.

From here we can go to **Finalize**.

* Next states: Finalize
* Errors:
    * Invalid unblock code
    * Unknown account
    * Incorrect password
    * Sign in with this email type is not currently supported
    * Failed to send email
* Available methods:
    * `VPNAuthInApp.setUnblockCodeAndContinue(code)`
    * `VPNAuthInApp.resendUnblockCodeEmail()`

### State: Totp verification needed

If the account is configured to use
[TOTP](https://en.wikipedia.org/wiki/Time-based_One-Time_Password), we are in
this state after the **Sign-in**. The client needs to inform the user and asks
for the 6-digit code.  From here, we can go to **Finalize**.

* Next states: Finalize
* Errors:
    * Invalid totp code
* Available methods:
    * `VPNAuthInApp.verifySessionTotpCode(code)`

### State: Sign-Up

In case we need to create a new account, we land in this state. In this state
we need to run a few steps:
- Ask for the password (this is the only thing we really care)
- Ask for other “legal” things (age, terms of service checkbox…)
- Validate the password:
    - The password cannot be a common one (there is a list from FxA)
    - The password must be at least 8 digits
    - The password cannot contain the account email address
- Create the account

The account creation requires email validation. The user receives a 6-digit
code via email. If this is needed, we go to **Email validation**.

* Next states: Email validation
* Errors:
    * Account already exists - rare
    * Email already exists - rare
* Available methods:
    * `VPNAuthInApp.validatePasswordCommons(password) -> bool`
    * `VPNAuthInApp.validatePasswordLength(password) -> bool`
    * `VPNAuthInApp.validatePasswordEmail(password) -> bool`
    * `VPNAuthInApp.setPassword(password)`
    * `VPNAuthInApp.signUp()`

### State: email validation

This state is super super similar to the “**unblock code needed**” one, but,
because this is a different FxA concept, we want to keep the 2 separate. The
behavior is exactly the same: the user needs to insert a 6-digit code and then
we go to **Finalize**.

If needed, the user can ask for a new session code email. See
`VPNAuthInApp.resendVerificationSessionCodeEmail()`.

* Next states: Finalize
* Errors:
    * Invalid unblock code
* Available methods:
    * `VPNAuthInApp.verifySessionEmailCode(code)`
    * `VPNAuthInApp.resendVerificationSessionCodeEmail()`

### State: Finalize

This is the last state if all work fine. The authentication is ready to be
finalized. This step contains a few network requests to obtain the session
code. After that, the authentication is finally completed.

Implementation-wise, this state does not exist, because, instead of landing
here, we dismiss the authentication component and we show the main VPN view.

* Next states: none
* Errors:
    * Authentication failure
* Available methods: none

### State: fallback in the browser

In case the authentication method is “without account creation”, we land here.
The authentication continues in the browser as we used to do in v2.7.

* Next states: none
* Errors:
    * Authentication failure
* Available methods: none

## Errors
- Account already exists
- Unknown account
- Incorrect password
- Invalid unblock code
- Email type not supported
- Email already exists
- Email can not be used to login
- Failed to send email
- Too many requests
- Server unavailable
- Invalid Totp code
