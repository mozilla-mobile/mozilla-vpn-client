# Glean on the Mozilla VPN application

This section of documentation deep dives into how [Glean](#2-Glossary) is integrated
on the Mozilla VPN application. It is aimed at developers working on the Mozilla VPN.

However, this is not a guide on how to add new data collection to the Mozilla VPN.
For a step-by-step guide on that refer to ["Working on tickets with Glean instrumentation"](#2-Glossary).
[The Glean book](#2-Glossary) is also the source of truth for Glean API reference documentation.
The Mozilla VPN follows the mozilla-central Glean integration implementation, so when looking at
Glean API docs on the book, choose the "Firefox Desktop" examples tab. The API will be the same.

> **Note**: An understanding of the fundamentals of Glean such as metrics, pings and glean_parser is assumed.

> **Note**: For a list of Glean features/metrics that are not yet implemented on the Mozilla VPN Glean integration,
> please consult [VPN-3683](https://mozilla-hub.atlassian.net/browse/VPN-3683).

## Background

On a first attempt to integrate Glean on the VPN application,
the [Glean JavaScript SDK](https://github.com/mozilla/glean.js/) was chosen, because QML provides a JavaScript runtime
and the JavaScript SDK could be used on QML mostly "out-of-the-box". Unfortunately the limitations
of having a data collection library based on the QML layer of the application outweigh the advantages.

**Telemetry APIs are only available on QML**. Signals and slots can be leveraged to
record telemetry from the backend of the application, however that requires aggressively wrapping
the telemetry library APIs. The Glean APIs are very robust and a lot of work is to required write
a complete wrapper for them. That invites simplification of these APIs. On the long run that puts
a significant limitation on what sort of data collection design can quickly be implemented.

**Inability to send telemetry when the application is backgrounded on mobile**. The QML layer
of the application is immediately killed when the mobile application is backgrounded. The VPN
application is especially susceptible to this issue, since it is a "set it and forget it" type
of application and the bulk of its usage happens while the app is on the background. Not only
is data collection not possible on the background, sending telemetry pings right as the application
is going to background is also not possible, because the ping uploader is most likely killed before
completing the uploads.

## Solution

Glean provides [a family of cross platform Rust based SDKs](#2-Glossary), including an iOS
and an Android SDK. These SDKs can be leveraged to collect telemetry from the backend
of the VPN client i.e. from the C++, Kotlin and Swift layers.

The main issue with using these SDKs, is that a C++/Qt SDK is not provided. The solution for
that is to write a wrapper around the Glean Rust SDK in order to call the Glean APIs from C++.
How is that different from writing a Glean wrapper based on the Glean JavaScript SDK?
Wouldn't it invite simplification of the Glean APIs just the same?

That _would_ be the case. However the Glean team has already written a very similar wrapper for
using Glean on C++, called [project FOG (Firefox on Glean)](#2-Glossary). Which means a full C++ wrapper
doesn't need to be implemented from scratch and instead FOG can largely be used as a base,
by removing the Firefox specific code and adding some Qt/QML sprinkles on top.

Finally, since the Glean mobile SDKs are based on the same Rust core as the Glean Rust SDK,
they can seamlessly be used alongside one another as longs as they are all linked to the exact same
Glean Rust core library -- read more on this on the "Adding Glean to mobile builds" section of this document.

## Architecture

The Glean integration code on the Mozilla VPN codebase can be found mostly under the `/qtglean`
folder plus a small extension on the `src/glean` folder.

```
├── src                      Bulk of the MozillaVPN source code
    └── glean                Calls on the FFI APIs exposed by qtglean, initializes Glean for the correct platform

├── qtglean                 Bulk of the Mozilla VPN Glean integration code

    ├── glean_parser_ext     Extends glean_parser to generate metrics and ping files for the MozillaVPN
        └── templates        Jinja templates for C++ and Rust metrics and pings files

    └── src                  Rust crate that calls on the Glean Rust SDK and exposes glean-core APIs
        ├── ffi              Metrics and ping FFI interface to be called by the generated files
        └── generated        Generated metrics and ping Rust files
```

### `qtglean/glean_parser_ext`

> **Note**: This part of the code is _heavily_ copy pasted from mozilla-central
> [`t/c/glean/build_scripts/glean_parser_ext`](#2-Glossary).

In order to be able to call Glean APIs from C++ and QML this glean_parser extension contains custom templates
and extends the functionalities of glean_parser to generate C++ and Rust metrics and pings files that work together.
The C++ templates are necessary, because glean_parser simply doesn't support C++. But why are new Rust templates also
necessary when that is already provided by glean_parser as a default?

These templates are different from usual glean_parser templates. Normally, glean_parser has templates that
simply instantiate each of the metrics and pings defined in the YAML files. The custom templates also do that,
however the custom Rust templates also generate a map where each metric or ping instance is assigned an index.
That index is then used to instantiate the metrics and pings in C++ and refer back to the Rust instance of the
metric in a very straight forward way, leaving all of the metric implementation complexity on the Rust side and
requiring a single integer to be passed from C++ through the FFI layer into Rust in order to access the right
metric or ping instance.

Let's look at an example. Take the following metric declaration:

```yaml
example:
  my_boolean:
    type: boolean
    lifetime: ping
    send_in_pings:
      - main
    description: |
      An example
    bugs:
      - https://github.com/mozilla-mobile/mozilla-vpn-client/issues/000
    data_reviews:
      - https://github.com/mozilla-mobile/mozilla-vpn-client/pull/001#c1
    data_sensitivity:
      - technical
    notification_emails:
      - brizental@mozilla.com
      - vpn-telemetry@mozilla.com
    expires: never
```

That will be translated by the default glean_parser Rust template into:

```rust
pub mod example {
    #[allow(non_upper_case_globals, dead_code)]
    /// generated from example.my_boolean
    ///
    /// An example
    pub static my_boolean: ::glean::private::__export::Lazy<BooleanMetric> = ::glean::private::__export::Lazy::new(|| {
        BooleanMetric::new(CommonMetricData {
            category: "example".into(),
            name: "my_boolean".into(),
            send_in_pings: vec!["main".into()],
            lifetime: Lifetime::Ping,
            disabled: false,
            ..Default::default()
        })
    });
}
```

That template makes it very easy to call `example::my_boolean` APIs from Rust,
but how to call those APIs from C++ through an FFI layer?

Let's now look at what the Rust and C++ generated code looks like, using the `glean_parser_ext` custom templates.

```rust
pub mod example {
    #[allow(non_upper_case_globals)]
    /// generated from example.my_boolean
    ///
    /// An example
    pub static my_boolean: Lazy<BooleanMetric> = Lazy::new(|| {
        BooleanMetric::new(CommonMetricData {
            name: "my_boolean".into(),
            category: "example".into(),
            send_in_pings: vec!["main".into()],
            lifetime: Lifetime::Ping,
            disabled: false,
            ..Default::default()
        })
    });
}

pub static BOOLEAN_MAP: Lazy<HashMap<MetricId, &Lazy<glean::private::BooleanMetric>>> = Lazy::new(|| {
    let mut map: HashMap<MetricId, &Lazy<glean::private::BooleanMetric>> = HashMap::with_capacity(1);
    map.insert(1.into(), &example::my_boolean);
    map
});
```

```cpp
namespace example {
    /**
     * generated from example.my_boolean
     */
    /**
     * An example
     */
    constexpr BooleanMetric my_boolean(1);

}
```

The metric maps make it easy to generate C++ objects that have a reference to the Rust instances of the metric
that is easy to pass through the FFI layer.

### `qtglean/src/ffi`

> **Note**: This part of the code is _heavily_ copy pasted from mozilla-central
> [`t/c/glean/api/src/ffi`](#2-Glossary).

The `ffi` module on the `qtglean` Rust crate exports all of the metrics and pings APIs to the FFI.
These are standalone functions which receive the metric id as a first argument to be able to call
the desired APIs on the correct metric instances.

Continuing on the above example, this is what the boolean [`set`](#2-Glossary) API looks like.

```rust
#[no_mangle]
pub extern "C" fn glean_boolean_set(id: u32, value: bool) {
    with_metric!(BOOLEAN_MAP, id, metric, metric.set(value));
}
```

Macros are used to reduce code repetition, but for the purposes of this document they are getting in the way.
Let's look at how that function would look like once the macro is expanded.

```rust
#[no_mangle]
pub extern "C" fn glean_boolean_set(id: u32, value: bool) {
    match crate::metrics::__glean_metric_maps::BOOLEAN_MAP.get(&id.into()) {
        Some(metric) => metric.set(value),
        None => panic!("No metric for id {}", $id),
    }
}

```

With the macro expanded we can see the `BOOLEAN_MAP` is queried using the metric id in order to access the concrete metric instance
and once that is found -- if it is found -- the metric API is called on it.

### `src/glean`

The `src/glean` module is where these exposed FFI functions are called.

Let's look at the C++ implementation of the `BooleanMetric`.

```cpp
class BooleanMetric final {
 public:
  constexpr explicit BooleanMetric(int aId) : m_id(aId) {}

  void set(bool flag) const { glean_boolean_set(m_id, flag) };

 private:
  int m_id;
};
```

It contains close to no logic at all. The only thing it keeps track of is the metric id,
provided once the metric instance is initialized on the glean_parser_ext generated files
and it calls on the FFI function exposed by Rust.

We have reached full circle. The boolean example is an intentionally simple API and metric type
to aid in grasping the concept of the metric map and the complexities it aims to address.
Other Glean APIs such as the testing APIs and other metric types can be more complex than this,
however the metric maps concept remains the same throughout.

## Adding Glean to mobile builds

The Glean mobile SDKs are essentially language bindings linked to the Glean core, a Rust library.
Understanding this architecture is important, because it is taken advantage of on the Mozilla VPN
to use Glean on C++, QML, Swift and Kotlin simultaneously and still record data and send pings
from the same instance of Glean.

Instead of consuming the Glean mobile SDKs through the normal channels, the `glean` repository is a
submodule of this repository. When building for mobile, the Glean platform specific code is added
to the Mozilla VPN sources -- it is worth noting that part of that code is generated using a tool
called [UniFFI](#2-Glossary) and Glean itself has internal metrics and pings which also require a generation step,
both generations steps are also added to the Mozilla VPN platform specific build setup.

The `qtglean` library exposes the glean_core library symbols as well as the symbols used by the Mozilla VPN C++ code.
Since that is linked to the Mozilla VPN project the Glean platform specific code added can access those symbols too.

With the Glean platform specific code and the Mozilla VPN linked to the same glean-core library,
any call to a Glean API from any language will record to the same Glean instance. Done.

## Note: Mobile Daemons

The daemons are responsible for managing the VPN tunnel and configuration. All platforms have
a main application / daemon architecture. In mobile however, the main application does not keep running
while backgrounded. This caveat is a big issue for telemetry.

A normal usage pattern on mobile is to open the application, turn on the VPN, background it and not
interact with it for a long time. All of this time of usage is not capture by the main application
Glean instance. Read more about this issue on [Overview and implications of scheduled task behavior on mobile devices](#2-Glossary)
and ["Mozilla VPN Telemetry Refactor](#2-Glossary).

To deal with that, the mobile daemons (Android Daemon and iOS Network Extension, to be more precise)
have a completely separate instance of Glean that permits telemetry collection while the application
is backgrounded. This choice of architecture means there is no need to deal with the complexities of passing
telemetry between processes -- the daemons each run in a separate process from the main application.

<!-- A single [`installation_id`](installation-id) metric is shared between the two processes for analyzer
to be able to join the data on the backend. TODO: Let's uncomment this one this metric is implemented. -->
-----
## Glossary 
[the-glean-book](https://mozilla.github.io/glean/book/index.html)
[readme-glean](https://github.com/mozilla-mobile/mozilla-vpn-client#working-on-tickets-with-new-glean-instrumentation)
[glean-js](https://mozilla.github.io/glean/book/language-bindings/javascript/index.html)
[glean-rs-sdks](https://mozilla.github.io/glean/book/language-bindings/index.html#rust-core-based-sdks)
[fog](https://firefox-source-docs.mozilla.org/toolkit/components/glean/index.html)
[t-c-glean-parser](https://searchfox.org/mozilla-central/source/toolkit/components/glean/build_scripts/glean_parser_ext)
[t-c-g-ffi](https://searchfox.org/mozilla-central/source/toolkit/components/glean/api/src/ffi)
[glean-bool-set](https://mozilla.github.io/glean/book/reference/metrics/boolean.html)
[uniffi](https://mozilla.github.io/uniffi-rs/)
[sarah's-doc](https://docs.google.com/document/d/1A2O_eACk0P3jDy0g2rRN5UTBOUj-iN-re6tVsRpxtDA/edit?usp=sharing)
[bea's-doc](https://docs.google.com/document/d/1jyNZ_g_cUpZZsEr2hYwnwZgkxlvqTmoFUJKp_LAOPts/edit?usp=sharing)
