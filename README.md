# Animation Creator Demo

## Dependencies
### SDKs in use (version)
* HOOPS Communicator (2025.5.0)
* HOOPS Exchange_Publish (2025.5.0)

### Tested server platforms
* Windows 11

## Setup
### Demo folders
&emsp;+ animation_creator<br>
&emsp;&emsp;+ css<br>
&emsp;&emsp;+ data<br>
&emsp;&emsp;+ HtmlConverter<br>
&emsp;&emsp;&emsp;+ bin <b>(copy all files from HOOPS_Exchange_Publish_SDK/bin/win64_v142)</b><br>
&emsp;&emsp;&emsp;+ template<br>
&emsp;&emsp;+ httpdServer<br>
&emsp;&emsp;+ js (copy hoops-web-viewer.mjs and engine.esm.wasm files from HOOPS_Communicator_SDK/web_viewer)<br>
&emsp;&emsp;+ json (copy hoops-web-viewer.mjs and engine.esm.wasm files from HOOPS_Communicator_SDK/web_viewer)<br>
&emsp;&emsp;+ model_data (copy hoops-web-viewer.mjs and engine.esm.wasm files from HOOPS_Communicator_SDK/web_viewer)<br>
&emsp;+ animation_creator.html<br>
&emsp;+ HttpServer.js<br>
&emsp;+ package.json<br>
&emsp;+ README.md<br>

## Start demo
1. Launch HTTP server with tarminal<br>
    `cd path/to/animation_creator`<br>
    `npm install`<br>
    `npm start`<br>
2. Start httpdServer (giving a port number in command line argument)<br>
    `cd path/to/animation_creator/httpdServer/bin`<br>
    `HttpdServer 8888`<br>
3. Open the main.html with server's port number (using Chrome)<br>
    `http://localhost:8000/animation_creator/animation_creator.html?viewer=SCS&instance=microengine&port=8888`
