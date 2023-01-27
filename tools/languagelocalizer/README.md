# Language string mapping tool

This tool retrieves the native language and the localized language names for
all the available locales.

## How to interact with this tool

During the execution, this tool uses `translations/languages.json` to retrieve
the WikiData resource code for all the available languages. If there is a new
language on Pontoon, it will ask for the code.  You can write the code or an
alternative one if wiki data does not have a page about that new language.

For instance, while I write this README, `es_CL` is an unknown language on wiki
data, and we use the `es` as an alternative code.

## How to find the code on wikidata

Search for the language code (fa - Persian), for instance. Open the page and
see the URL: https://www.wikidata.org/wiki/Q9168. The code is Q9168.
