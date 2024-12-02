# Replacer

Replacer addons can be used to replace QRC resources at execution time. The addon must include all the required files in its bundle.

The JSON files are validated via [JSON Schema](https://json-schema.org/) files:

https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/scripts/ci/jsonSchemas

On this page, we want to describe how to write a replacer addon.

### Considerations

 * Replacing QML files can be hazardous! The application will break if the QMLs are broken, or the dependencies are unsatisfied.

 * Replacing an entire folder is even more dangerous! Image what happens if you replace the whole `i18n` folder with broken translation files.

## JSON format

The Replacer JSON files are addon JSON objects with the type "replacer" and a
"replacer" property object with the following properties:

| Property | Description  | Type | Required |
| -- | -- | --| -- |
| urls | An array of Replace objects | Array of objects | Yes |

### Replace object

This object must have the following structure:

| Property | Description  | Type | Required |
| -- | -- | --| -- |
| request | The QRC URL to be replaced | string | Yes |
| response | The relative path of the file/directory to use to replace the `request` | string | Yes |
| type | The type of the request (`file` or `directory`). By default: `file` | string | No |
