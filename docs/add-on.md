# Add-ons

The add-on system is the basic component for dynamic contents for the Mozilla VPN client.
Guides and messages are add-ons and they are dynamically loaded from a trusted source (archive.mozilla.org).

## Add-on index and the signature verification

The VPN client downloads the add-on index file and its signature from a trusted
source. These 2 files are stored on disk for a quick loading of the add-ons at startup.

The signature of the add-on index file is verified at any loading. We use RSA
4096 + SHA256 as sign algorithm.

The add-on index file is a JSON array containing add-on IDs and hash (SHA256)
of the add-on content. The app downloads the add-ons listed in the index file
if they are not on disk already or if the hash does not match.

## Add-on format

Add-ons are [RCC](https://doc.qt.io/qt-6/resources.html) files containing at
least a manifest.json file. The properties of this JSON file are:


| Property | Description | Type | Required |
| --- | --- | --- | --- |
| id | The ID of the add-on. It must match the file name | String | Yes |
| name | The name of the add-on | String | Yes |
| api_version | The version of the add-on framework | String | Yes |
| type | One of the supported types (message, guide, ...) | String | Yes |
| conditions | List of conditions to meet | Array of Condition objects | No |
| state | Object describing the state of the addon | Collection of state objects | No |

Based on the add-on type, extra properties can be in added. See the
[guide](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/guides.md), [message](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/message.md), and [replacer](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/replacer.md) documentation.

## State Object

An addon may have three different types of state: `session`, `local` or `global`.

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| session | An object describing the addons session state | State | No |
| local | An object describing the addons local state | State | No |
| global | An object describing the addons global state | State | No |

The `State` interface is the same, regardless of the type of state.
There is no limit of properties for a state, however the only supported state types are `string`, `number` and `boolean`.
The state object may _not_ be nested.

Following is the state interface, described using Typescript notation.

```ts
interface State {
    [key: string]: {
        type: “string” | “number” | “boolean”,
        default: string | number | boolean,
    }
}

```

### Session state

Session state is _not_ persisted throughout user sessions i.e. it is wiped once the application is killed.

### Local state

> **Note**: Local state is not supported yet, if you need to implement an addon that requires it please refer to [VPN-3929](https://mozilla-hub.atlassian.net/browse/VPN-3929).

Local state is persisted throughout user sessions i.e. not wiped after the application is killed. It is _not_ synced among the devices of a given user. It is local to each device.

### Global state

> **Note**: Global state is not supported yet, if you need to implement an addon that requires it please refer to [VPN-2795](https://mozilla-hub.atlassian.net/browse/VPN-2795).

Global state is persisted throughout user sessions and synced among user devices.


## Condition object

Add-ons can enable and disable themselves using the `conditions` key in the manifest. The condition object contains the following properties:

| Property | Description | Type | Required | Dynamic |
| --- | --- | --- | --- | --- |
| enabled_features | An array of features to be enabled | Array of string | No | No |
| env | A string to match a particular env: staging, production | String | No | No |
| locales | An array of locales to be checked | Array of string | No | Yes |
| min_client_version | The min client version | String | No | No |
| max_client_version | The max client version | String | No | No |
| platforms | An array of platforms to be checked | Array of string | No | No |
| settings | An array of Condition Setting object. See below | Array of Condition Setting object | No | No |
| trigger_time | A number identifying the number of seconds from the first execution of the client | Integer |  No | Yes |
| start_time | The epoch time that activates the current add-on | Integer | No | Yes |
| end_time | The epoch time that deactivates the current add-on | Integer | No | Yes |
| javascript | A script file is executed to change the condition status. See below | String | No | Yes |
| translation_threshold | The translation threshold to use. By default 1 (full translation required) | Number | No | No |

Some conditions are dynamic. This means that the value can change their status dynamically during the app execution.

The add-on is considered enabled if all the conditions are met.

### Condition Setting object

When a setting must be checked as a condition, the JSON object must contain the following properties:

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| setting | The name of the setting key | String | Yes |
| value | The value of the setting key | String | Yes |
| op | The compare operator: eq or neq | String | Yes |

The list of setting keys can be found here: https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/src/settingslist.h

### Javascript conditions

When the add-on manifest contains a `javascript` property in the `conditions` object, its value must be a javascript filename.

The javascript file is executed when the add-on is loaded and it has to expose a function. For instance:

```
(function(api, condition) {
  // your code goes here.
})
```

The function will be executed at the first add-on loading with 2 parameters.

 * api: the [add-on API object](https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/docs/add-on-api.md).
 * condition: a `condition` object.

The `condition` object exposes 2 methods:

 * `condition.enable()` - to be called to enable the add-on
 * `condition.disable()` - to be called to disable the add-on

An example of javascript conditional object is the following:

```
(function(vpn, condition) {
  const xhr = new XMLHttpRequest();
  xhr.open('GET', 'https://mozilla.org');
  xhr.send();
  xhr.onreadystatechange = () => {
    if (xhr.readystate === 4) condition.enable()
  }
})
```

## How to implement and test add-ons

If you want to implement new add-ons, you need to follow these steps:

1. Create a manifest in a separate folder in the `addons` directory of the mozilla VPN repository.
2. Read and follow the documentation for the add-on type you want to implement.
3. Build the CMake project in the `addons` directory.

```
mkdir -p build-addons/
cmake -S <source>/addons -B build-addons -GNinja
cmake --build build-addons
```

4. Expose the generated build directory through a webservice. For example: `python3 -m http.server --directory build-addons/`
5. Open the dev-menu from the get-help view and set a custom add-on URL: `http://localhost:8000/`
6. Scroll down and disable the signature-addon feature from the dev-menu, list of features
7. Be sure you are doing all of this using a staging environment

If all has done correctly, you can see the app fetching the manifest.json (and
not! the manifest.json.sig) resource from the webservice.
