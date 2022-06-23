# Guides

Guides are part of the "Tips & Tricks" page and they are a way to inform the
users about features, HowTo, and functionalities of the Mozilla VPN app.

Technically, they are implemented as add-ons (see the add-on documentation).

The JSON files are validated via [JSON Schema](https://json-schema.org/) files:
https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/scripts/ci/jsonSchemas

On this page, we want to describe how to write new guides.

## JSON format

The Guide JSON files are add-on JSON objects, with type "guide" and a "guide"
property object with the following properties:


| Property | Description | Type | Required |
| --- | --- | --- | --- |
| id | Each guide must have an id. This is used for the localization | String | Yes |
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
