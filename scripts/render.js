import { renderLoopHTML } from './renderHTML.js';
import { renderLoopCanvas } from './renderCanvas.js';

export function renderLoop(currentTime, memoryDataView, scratchSpaceAddress, renderCommandSize, htmlRoot, canvasRoot, canvasContext, textDecoder, instance, imageCache, elementCache, previousFrameTime) {
    const elapsed = currentTime - previousFrameTime;
    previousFrameTime = currentTime;
    let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);

    if (activeRendererIndex === 0) {
        instance.exports.UpdateDrawFrame(scratchSpaceAddress, window.innerWidth, window.innerHeight, 0, 0, window.mousePositionXThisFrame, window.mousePositionYThisFrame, window.touchDown, window.mouseDown, 0, 0, window.dKeyPressedThisFrame, elapsed / 1000);
    } else {
        instance.exports.UpdateDrawFrame(scratchSpaceAddress, window.innerWidth, window.innerHeight, window.mouseWheelXThisFrame, window.mouseWheelYThisFrame, window.mousePositionXThisFrame, window.mousePositionYThisFrame, window.touchDown, window.mouseDown, window.arrowKeyDownPressedThisFrame, window.arrowKeyUpPressedThisFrame, window.dKeyPressedThisFrame, elapsed / 1000);
    }

    let rendererChanged = activeRendererIndex !== window.previousActiveRendererIndex;
    switch (activeRendererIndex) {
        case 0: {
            renderLoopHTML(memoryDataView, scratchSpaceAddress, renderCommandSize, htmlRoot, elementCache, textDecoder, instance);
            if (rendererChanged) {
                htmlRoot.style.display = 'block';
                canvasRoot.style.display = 'none';
            }
            break;
        }
        case 1: {
            renderLoopCanvas(memoryDataView, scratchSpaceAddress, renderCommandSize, canvasRoot, canvasContext, textDecoder, instance, imageCache);
            if (rendererChanged) {
                htmlRoot.style.display = 'none';
                canvasRoot.style.display = 'block';
            }
            break;
        }
    }

    window.previousActiveRendererIndex = activeRendererIndex;
    requestAnimationFrame((time) => renderLoop(time, memoryDataView, scratchSpaceAddress, renderCommandSize, htmlRoot, canvasRoot, canvasContext, textDecoder, instance, imageCache, elementCache, previousFrameTime));
    window.mouseDownThisFrame = false;
    window.arrowKeyUpPressedThisFrame = false;
    window.arrowKeyDownPressedThisFrame = false;
    window.dKeyPressedThisFrame = false;
}
