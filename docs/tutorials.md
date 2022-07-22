# Tutorials

Tutorials are part of the "Tips & Tricks" page and they are a way to teach how
to use the Mozilla VPN features with an interactive system.

Technically, they are implemented as add-ons (see the add-on documentation).

The JSON files are validated via [JSON Schema](https://json-schema.org/) files:
https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/scripts/ci/jsonSchemas

On this page, we want to describe how to write new tutorials.

## JSON format

The Tutorial JSON files are add-on JSON objects, with type "tutorial" and a "tutorial"
property object with the following properties:

| Property                   | Description                                                                                                  | Type                  | Required |
| -------------------------- | ------------------------------------------------------------------------------------------------------------ | --------------------- | -------- |
| id                         | Each tutorial must have an id. This is used for the localization                                             | String                | Yes      |
| highlighted                | Is this tutorial highlighted?                                                                                | Boolean               | No       |
| advanced                   | Is this an advanced tutorial? Defaults to `false`. This property is ignored in case `highlighted` is `true`. | Boolean               | No       |
| title                      | The title of the Tutorial                                                                                    | String                | Yes      |
| title_comment              | An optional comment to describe the meaning of the title                                                     | String                | No       |
| subtitle                   | The subtitle of the Tutorial                                                                                 | String                | Yes      |
| subtitle_comment           | An optional comment to describe the meaning of the subtitle                                                  | String                | No       |
| completion_message         | The completion message to show at the end of the Tutorial                                                    | String                | Yes      |
| completion_message_comment | An optional comment to describe the meaning of the completion message                                        | String                | No       |
| image                      | The URL (qrc:) of the image                                                                                  | String                | Yes      |
| steps                      | An array of steps. See below                                                                                 | Array of Step objects | Yes      |

### Step object

Each tutorial is composed of a list of steps (tooltips).

A step is a JSON object composed of the following properties:

| Property   | Description                                                                      | Type                   | Required |
| ---------- | -------------------------------------------------------------------------------- | ---------------------- | -------- |
| id         | Each step must have an id. This is used for the localization                     | String                 | Yes      |
| tooltip    | The text content of the tooltip for this step                                    | String                 | Yes      |
| comment    | An optional comment to describe the meaning of the tooltip content               | String                 | No       |
| element    | The object name of the QML element to be used for the positioning of the tooltip | Yes                    |
| conditions | A list of conditions to be checked before exposing this step. See below          | Condition object       | No       |
| before     | An array of operations to run before showing the tooltip                         | Array of Before object | No       |
| next       | An object to describe when this step has to be considered complete               | Next object            | Yes      |

### Condition object

The condition object is similar to the add-on one. See the Add-on documentation.

### Before object

The Before object describe an operation to be done before showing the tooltip. Its properties are:

| Property | Description                                                                     | Type   | Required |
| -------- | ------------------------------------------------------------------------------- | ------ | -------- |
| op       | The name of the operation. Each operation has its own sub-properties. See below | String | Yes      |

#### Before object operations

The list of operations are:

##### property_set

Set a property value to a QML object. It's properties are:

| Property | Description                  | Type   | Required |
| -------- | ---------------------------- | ------ | -------- |
| element  | The QML object name          | String | Yes      |
| property | The property name            | String | Yes      |
| value    | The property value to be set | any    | Yes      |

##### property_get

Get and compare a property value of a QML object. It's properties are:

| Property | Description                 | Type   | Required |
| -------- | --------------------------- | ------ | -------- |
| element  | The QML object name         | String | Yes      |
| property | The property name           | String | Yes      |
| value    | The property value to match | any    | Yes      |

##### vpn_location_set

Change the VPN location. It's properties are:

| Property         | Description            | Type   | Required              |
| ---------------- | ---------------------- | ------ | --------------------- |
| exitCountryCode  | The exit country code  | String | Yes                   |
| exitCity         | The exit city name     | String | Yes                   |
| entryCountryCode | The entry country code | String | No (Yes for multihop) |
| entryCity        | The entry city name    | String | No (Yes for multihop) |

### Next object

The next object describes how this step has to be considered complete. It's a JSON object with the following properties:

| Property | Description                                                                     | Type   | Required |
| -------- | ------------------------------------------------------------------------------- | ------ | -------- |
| op       | The name of the operation. Each operation has its own sub-properties. See below | String | Yes      |

### Next object operations

So far we support only one operation: 'signal'. This operation requires the following properties:

| Property    | Description                                                                                                        | Type   | Required                    |
| ----------- | ------------------------------------------------------------------------------------------------------------------ | ------ | --------------------------- |
| qml_emitter | A QML object name that has to emit a signal                                                                        | String | Yes if vpn_emitter is empty |
| vml_emitter | An internal VPN component that has to emit a signal. So far, we support only: **settingHolder** and **controller** | String | Yes if qml_emitter is empty |
| signal      | The signal name                                                                                                    | String | Yes                         |
