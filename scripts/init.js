import { initializeWasm, getMemoryDataView, getScratchSpaceAddress, getRenderCommandSize, getInstance } from './wasm.js';
import { renderLoop } from './render.js';
import { initializeUtils,setupEventListeners } from './utils.js';

let previousFrameTime;
let elementCache = {};
let imageCache = {};

async function init() {
    await initializeWasm();
    initializeUtils();
    setupEventListeners();
    
    // Initialize HTML and Canvas roots
    window.htmlRoot = document.body.appendChild(document.createElement('div'));
    window.canvasRoot = document.body.appendChild(document.createElement('canvas'));
    window.canvasContext = window.canvasRoot.getContext("2d");

    // Initialize event listeners and other setup...

    // Start the render loop
    requestAnimationFrame((time) => renderLoop(
        time,
        getMemoryDataView(),
        getScratchSpaceAddress(),
        getRenderCommandSize(),
        window.htmlRoot,
        window.canvasRoot,
        window.canvasContext,
        new TextDecoder("utf-8"),
        getInstance(),
        imageCache,
        elementCache,
        previousFrameTime
    ));
}

init();