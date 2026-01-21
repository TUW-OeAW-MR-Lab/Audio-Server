# WebGUI

Buttons on the left side: Scenes

* Each button loads a HTML file from the directory scenes

## Scenes

* Must start with `<div id="%SceneName%" class="scene-main">` and end with `</div>`.

* No scripts allowed. If scripts are required: 
  * Create a JS file in `scripts/%SceneName%.js`.
	* Use `export function` instead of `function`, 
	* Include a function called `init`. This function is obligatory and will be executed before the scene will be shown. 
	* Functions within that script can be accessed as `SceneModule.%someFunction%`, e.g., `SceneModule.init()`. 

* The highest level of header must be `<h2>%SceneName%</h2>`.

* To start an app: ` <button onClick="launchApp(%SceneName%, %appName%)">Launch App</button>`. Server responses with the following states: 
 * `alive`: command acknowledged, launching the app.
 * `ready`: App launched. Now commands can be sent to the app.
 * `success`: ??? (appears when the App has been closed by the admin).

* To start an app and wait until the app has been launched: ` <button onClick="launchAppAndWait(%SceneName%, %app-name%, %timeout%)">Launch App</button>`, which waits %timeout% seconds before creating an error. 

* To start an app, wait, and do some processing: ` <button onClick="launchAppAndWait(%SceneName%, %app-name%, %timeout%, SceneModule.init, SceneModule.ready)">Launch App</button>`, which calls the scene-related `init()` function before launching the `%app-name%` and `ready()` after the app has been successfully launched. 

* To quit an app: ` <button onClick="quitApp(%SceneName%, %appName%)">Quit App</button>. Server responses with the following states: 
 * `quitting...`: command acknowledged, quitting the app.
 * `killed`: app killed.
 * `error`: ??? (appears when the App has been killed).
 
* To quit an app and execute a function right after the quit command has been sent to the server: `quitApp(%SceneName%, %appName%, SceneModule.%quitFunction%)`.

* To connect to the app for a response channel: ` <button onClick="sendResponse('/app/%appName%/%pathToEstablishTheResponse%')">Establish Response</button> `.

* To send a command: ` <button onClick="sendNoArgs('/app/%appName%/%command%')">Send %command%</button> `.

* To send a command with a value: `<input type=range oninput="send('/app/pd-fly/osc/Control/Volume', this.value)">`

* To receive a state of a %variable% from the app, create an element such as: ` <span id="%appName%_%variable%"></span> `. Currently, this works only for %appName% is pd-fly or pd-jungle and only for variables structured as /Control/%variable%. 

* To receive the state of an app being loaded or killed, receive the state of the variable called `status`.