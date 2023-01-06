# Clone the stock remote
If you plug in the USB to anything that is able to power the dongle, the dongle will stay in clone mode for 5 seconds, if you press button 1, 2 or 3 of your stock remote within this timeframe, it will be cloned by the dongle and it should be able to control the FAN.
The onboard led will lit during “clone mode” and will turn off after the 5sec or immediately when successfully cloned.

# Testing the Dongle in Windows
To check whether the Dongle works ok, and if you are able to clone your stock remote, connect the Dongle to the USB port of your computer and open the App: “Windows Remote Arduino Experience”. Click on the “FanX RF Dongle” (USB Connection) and click connect. Now, open the tab “Analog and enable “Pin A0″, change the FAN speed with your stock remote and check if the value changes, it can take a few seconds before it updates the value.”

# Setting up Home Assistant

## Install the custom integration
Before changing anything in Home Assistant, it is always a good idea to make a backup. Then you should add the script shown on the left to the yaml configuration.
After this modification, you should reboot Home Assistant and then navigate to “Settings”, “Integrations”. Here you should see the new FanX Firmata integration. If you click on the Dongle, you should be able to see the FAN state if the remote was cloned already.
