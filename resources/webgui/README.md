# WebGUI

Buttons on the left side: Scenes

* Each button loads a HTML file from the directory scenes

## Scenes

* Must start with `<div id="YourSceneName" class="scene-main">`

* Must end with `</div>`

* The highest level of header must be `<h2>Your Scene</h2>`.

* To start an app: ` <button onClick="launchApp(YourSceneName, YourAppName)">Launch App</button>`.

* To quit an app: ` <button onClick="quitApp(YourSceneName, YourAppName)">Quit App</button>.

* To connect to the app for a response channel: ` <button onClick="sendResponse('/app/YourAppName/YourPathToEstablishTheResponse')">Establish Response</button> `.

* To send a command: ` <button onClick="sendNoArgs('/app/YourAppName/YourCommand')">Send YourCommand</button> `.

* To receive a state of a variable from the app, create an element such as: ` <span id="YourAppName_YourVariable"></span> `. Currently, this works only for YourAppName is pd-fly or pd-jungle and only for variables structured as /Control/YourVariable. 

* To receive the state of an app being loaded or killed, receive the state of the variable called `status`.