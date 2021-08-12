/*
 * settings.html.h
 */

#ifndef settings_html_h
#define settings_html_h

const char settings_html[] PROGMEM = R"(<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Settings</title><link rel="stylesheet" href="style.css"><link rel="shortcut icon" href="favicon.ico" type="image/x-icon"><script src="settings.js" defer></script><style>html {background-color: black;color: white;font-family: sans-serif;margin: 0;}pre {display: inline-block;margin: 0;}input[type=checkbox] {margin: 3px 6px 3px 0;}input[type=number] {width: 3ex;appearance: textfield;text-align: center;}input[type=range] {width: 100%;max-width: 256px;vertical-align: middle;}input[type=number]#impDur {width: 5ex;}</style><noscript><style>#impDur_alt {display: none;}</style></noscript></head><body><h1>Settings</h1><noscript><h2>note - inputs can not be pre-filled when JavaScript is not available.</h2></noscript><form action="" method="post"><div><h2>connect to WiFi using</h2><div><label for="sta_ssid">SSID:</label><br><input type="text" name="sta_ssid" id="sta_ssid" value="" maxlength="31" required></div><div><label for="sta_psk">PSK: (keeps old when unset)</label><br><input type="password" name="sta_psk" id="sta_psk" value="" minlength="8" maxlength="63"></div><div><input type="checkbox" name="sta_dhcp" id="sta_dhcp" checked><label for="sta_dhcp">use DHCP</label></div><div><label for="ip0">IP-address:</label><br><input type="number" name="ip0" id="ip0" value="0" min="0" max="255"><pre>.</pre><input type="number" name="ip1" id="ip1" value="0" min="0" max="255"><pre>.</pre><input type="number" name="ip2" id="ip2" value="0" min="0" max="255"><pre>.</pre><input type="number" name="ip3" id="ip3" value="0" min="0" max="255"></div><div><label for="gw0">gateway-address:</label><br><input type="number" name="gw0" id="gw0" value="0" min="0" max="255"><pre>.</pre><input type="number" name="gw1" id="gw1" value="0" min="0" max="255"><pre>.</pre><input type="number" name="gw2" id="gw2" value="0" min="0" max="255"><pre>.</pre><input type="number" name="gw3" id="gw3" value="0" min="0" max="255"></div><div><label for="sm0">subnet-mask:</label><br><input type="number" name="sm0" id="sm0" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sm1" id="sm1" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sm2" id="sm2" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sm3" id="sm3" value="0" min="0" max="255"></div><div><label for="pd0">primary DNS:</label><br><input type="number" name="pd0" id="pd0" value="0" min="0" max="255"><pre>.</pre><input type="number" name="pd1" id="pd1" value="0" min="0" max="255"><pre>.</pre><input type="number" name="pd2" id="pd2" value="0" min="0" max="255"><pre>.</pre><input type="number" name="pd3" id="pd3" value="0" min="0" max="255"></div><div><label for="sd0">secondary DNS:</label><br><input type="number" name="sd0" id="sd0" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sd1" id="sd1" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sd2" id="sd2" value="0" min="0" max="255"><pre>.</pre><input type="number" name="sd3" id="sd3" value="0" min="0" max="255"></div></div><div><h2>Logic</h2><div><input type="checkbox" name="flipHighLow" id="flipHighLow"><label for="flipHighLow">flip <pre>HIGH</pre> and <pre>LOW</pre></label></div><div><input type="checkbox" name="pin2asIn" id="pin2asIn"><label for="pin2asIn">use pin <pre>2</pre> as an input (use interrupt)</label></div><div><label for="interruptOn">interrupt on:</label><select name="interruptOn" id="interruptOn" required><option value="">--</option><option value="r">RISING</option><option value="f">FALLING</option><option value="c">CHANGE</option></select></div><div><input type="checkbox" name="sendImp" id="sendImp"><label for="sendImp">send an impulse instead of setting pins to <pre>HIGH</pre></label></div><div><label for="impDur">impulse duration in <pre>ms</pre>:</label><br><input type="number" name="impDur" id="impDur" value="0" min="0" max="512" step="1" oninput="impDur_alt.value=this.value" required><input type="range" name="impDur_alt" id="impDur_alt" value="0" min="0" max="512" step="1" oninput="impDur.value=this.value"></div></div><br><input type="reset" onclick="prefill() "><input type="submit"></form></body></html>
)";

#endif
