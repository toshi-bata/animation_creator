# Animation Creator Demo
## Overview
This demo showcases how to build a simple animation creation application using HOOPS Communicator. It is designed for use cases such as creating work instructions, simulating object assembly or disassembly, and more.

To operate this demo, we have implemented additional UI components that allow users to interact with and edit an animation timeline. You can remove keyframes, adjust the duration of animations, and modify the distance and angle a node moves or rotates. The animation timeline also supports adding elements such as camera movements, node translations, rotations, visibility changes, and part blinking to highlight specific components during playback.

In addition, the created animations can be exported to a Monolithic HTML file using the included HOOPS Publish HTML Converter. This enables easy sharing and viewing of animated instructions without requiring a separate server or SDK.

## Dependencies
### SDKs in Use (Version)
* HOOPS Communicator: 2025.5.0
* HOOPS Exchange_Publish: 2025.5.0

### Tested Server Platforms
* Windows 11
* Ubuntu Server 24.04 LTS (AWE EC2 instance)

## Setup
### Demo folders
animation_creator/              # Root folder<br>
├── css/                        <br>
├── data/                       # PRC model files<br>
├── HtmlConverter/<br>
│   ├── bin/                    # Copy all files from HOOPS_Exchange_Publish_SDK/bin/win64_v142 or linux64<br>
│   │   └── Export3DToHtml      # Monolithic HTML converter executable<br>
│   └── template/<br>
├── httpdServer/<br>
│   └── bin/<br>
│       └── HttpdServer         # HTTP daemon executable for the monolithic HTML converter<br>
├── js/                         # Copy hoops-web-viewer.mjs and engine.esm.wasm from HOOPS_Communicator_SDK/web_viewer<br>
│   └── animation_creator.js    # Main JavaScript<br>
├── json/<br>
├── model_data/<br>
├── animation_creator.html      # Main HTML page<br>
├── HttpServer.js<br>
└── package.json<br>

## Starting the Demo
1. Open a terminal and navigate the demo root folder:<br>
    `cd path/to/animation_creator`<br>
2. Launch httpdServer:<br>
    `npm install`<br>
    `npm start`<br>
3. Open a separate terminal<br>
4. Set the library path (Linux only)<br>
    `export LD_LIBRARY_PATH=/usr/local/lib:/path/to/Exchnage/bin`<br>
5. Start the HTTP daemon with a port number (e.g., 8888):<br>
    `cd path/to/animation_creator/httpdServer/bin`<br>
    `HttpdServer 8888`<br>
6. Access the main HTML page with the port number (in Chrome):<br>
    `http://localhost:8000/animation_creator/animation_creator.html?viewer=SCS&instance=microengine&port=8888`
