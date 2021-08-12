/*
 * settings.js.h
 */

#ifndef settings_js_h
#define settings_js_h

const char settings_js[] PROGMEM = R"(
async function fetchConfig() {
const response = await fetch('config')
return response.json()
}

async function prefill() {
config = await fetchConfig()
configValue = config['value']
configChecked = config['checked']

Object.keys(configValue).forEach((key) => {document.getElementById(key).value = configValue[key]})
Object.keys(configChecked).forEach((key) => {document.getElementById(key).checked = configChecked[key]})
}

prefill()
)";

#endif

