pebble_pc_automation
====================

Pebble watchapp and C# HTTP server to facilitate PC keyboard button presses from Pebble.

Usage:
1) Run the server on a Windows PC. Requires .NET 4.0. Debug messages will show in the same console window.
2) Install the pebble app using SDK 2.0. (The initial commit for watchapp was written using SDK 2.0 BETA 4).
3) Config the watchapp (server host IP and port) using via the Pebble phone app. (The config page is hosted here: https://googledrive.com/host/0B4gs1j2cF1C0SkNnVm54OUoyenc/pebble_pc_automation.html)
4) Launch watchapp. You should see the server IP and port in the Settings menu item subtitle.

Debug messages from the server side (requests and responses) show up in the server console window.
Debug messages on the watchapp (send status and errors from the server) show up in the subtitle section of the selected menu item.
