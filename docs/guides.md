# Guides

Guides are part of the "Tips & Tricks" page and they are a way to inform the
users about features, HowTo, and functionalities of the Mozilla VPN app.

Technically, they are implemented as JSON files, stored in the `qrc://guides`
folder.

The JSON files are validated via [JSON Schema](https://json-schema.org/) files:
https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/scripts/ci/jsonSchemas

On this page, we want to describe how to write new guides.

## JSON format

The Guide JSON files are JSON objects. The required properties are:


| Property | Description | Type | Required |
| --- | --- | --- | --- |
| id | Each guide must have an id. This is used for the localization | String | Yes |
| conditions | A list of conditions to be checked before exposing the guide. See below | Condition object | No |
| title | The title of the Guide | String | Yes |
| title_comment | An optional comment to describe the meaning of the title | String | No |
| subtitle | The subtitle of the Guide | String | Yes |
| subtitle_comment | An optional comment to describe the meaning of the subtitle | String | No |
| image | The URL (qrc:) of the image | String | Yes |
| blocks | An array of graphical blocks. See below | Array of Block objects | Yes |

### Block object

Each guide view is composed of a list of graphical blocks: titles, paragraphs, lists, etc.

A block is a JSON object composed of the following properties:

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| id | Each block must have an id. This is used for the localization | String | Yes |
| content | The text content of the block it can be a string or an array of objects. See below | String or array of Content objects | Yes |
| comment | An optional comment to describe the meaning of the content | String | No | 
| type | The type of the block. Possible values: text, title, ulist, olist | String | Yes |

### Content object

If the content is an object, it must contains the following properties:

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| id | Each content must have an id. This is used for the localization | String | Yes |
| content | The text content of the sub block | String | Yes |
| comment | An optional comment to describe the meaning of the content | String | No | 

### Condition object

The condition object contains the following properties:

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| enabledFeatures | An array of features to be enabled | Array of string | No |
| platforms | An array of platforms to be checked | Array of string | No |
| settings | An array of Condition Setting object. See below | Array of Condition Setting object | No |

### Condition Setting object

When a setting must be checked as a condition, the JSON object must contain the following properties:

| Property | Description | Type | Required |
| --- | --- | --- | --- |
| setting | The name of the setting key | String | Yes |
| value | The value of the setting key | String | Yes |
| op | The compare operator: eq or neq | String | Yes |

The list of setting keys can be found here: https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/src/settingslist.h
