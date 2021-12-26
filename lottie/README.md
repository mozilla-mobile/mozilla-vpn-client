# Lottie for QML

This is the MozillaVPN QML Lottie animation item. It is built on top of
[lottie-web](https://github.com/airbnb/lottie-web) and a QML Canvas element.

### lottietest
If you want to test this component, you can run the `lottietest` app passing a
lottie JSON file as the first argument:

```
# ./bin/lottietest tests/qml/test.json
```

### How to use it

See the LottieAnimation.qml file for a good description of properties and
methods. Currently, we support the minimum set of features required for the
Mozilla VPN project.

### Tests

This component is released with 2 types of tests:

- unit-tests: `./tests/unit/tests`
- qml-tests: `./tests/qml/tst_lottie`
