import { getMemoryDataView, getInstance } from './wasm.js';


let memoryDataView;
let instance;

export function initializeUtils() {
    memoryDataView = getMemoryDataView();
    instance = getInstance();
}

export function MemoryIsDifferent(one, two, length) {
    for (let i = 0; i < length; i++) {
        if (one[i] !== two[i]) {
            return true;
        }
    }
    return false;
}

export function getStructTotalSize(definition) {
    switch (definition.type) {
        case 'union':
        case 'struct': {
            let totalSize = 0;
            for (const member of definition.members) {
                let result = getStructTotalSize(member);
                if (definition.type === 'struct') {
                    totalSize += result;
                } else {
                    totalSize = Math.max(totalSize, result);
                }
            }
            return totalSize;
        }
        case 'float': return 4;
        case 'uint32_t': return 4;
        case 'uint16_t': return 2;
        case 'uint8_t': return 1;
        case 'bool': return 1;
        default: {
            throw "Unimplemented C data type " + definition.type;
        }
    }
}

export function readStructAtAddress(address, definition) {
    switch (definition.type) {
        case 'union':
        case 'struct': {
            let struct = { __size: 0 };
            for (const member of definition.members) {
                let result = readStructAtAddress(address, member);
                struct[member.name] = result;
                if (definition.type === 'struct') {
                    struct.__size += result.__size;
                    address += result.__size;
                } else {
                    struct.__size = Math.max(struct.__size, result.__size);
                }
            }
            return struct;
        }
        case 'float': return { value: memoryDataView.getFloat32(address, true), __size: 4 };
        case 'uint32_t': return { value: memoryDataView.getUint32(address, true), __size: 4 };
        case 'uint16_t': return { value: memoryDataView.getUint16(address, true), __size: 2 };
        case 'uint8_t': return { value: memoryDataView.getUint8(address, true), __size: 1 };
        case 'bool': return { value: memoryDataView.getUint8(address, true), __size: 1 };
        default: {
            throw "Unimplemented C data type " + definition.type;
        }
    }
}

export function getTextDimensions(text, font) {
    window.canvasContext.font = font;
    let metrics = window.canvasContext.measureText(text);
    return { width: metrics.width, height: metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent };
}

export function setupEventListeners() {
    window.mousePositionXThisFrame = 0;
    window.mousePositionYThisFrame = 0;
    window.mouseWheelXThisFrame = 0;
    window.mouseWheelYThisFrame = 0;
    window.touchDown = false;
    window.mouseDown = false;
    window.mouseDownThisFrame = false;
    window.arrowKeyDownPressedThisFrame = false;
    window.arrowKeyUpPressedThisFrame = false;
    window.dKeyPressedThisFrame = false;

    // Mouse move event
    document.addEventListener("mousemove", (event) => {
        let target = event.target;
        let scrollTop = 0;
        let scrollLeft = 0;
        let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);
        while (activeRendererIndex !== 1 && target) {
            scrollLeft += target.scrollLeft;
            scrollTop += target.scrollTop;
            target = target.parentElement;
        }
        window.mousePositionXThisFrame = event.x + scrollLeft;
        window.mousePositionYThisFrame = event.y + scrollTop;
    });

    // Mouse down event
    document.addEventListener("mousedown", (event) => {
        window.mouseDown = true;
        window.mouseDownThisFrame = true;
    });

    // Mouse up event
    document.addEventListener("mouseup", (event) => {
        window.mouseDown = false;
    });

    // Touch events
    function handleTouch(event) {
        if (event.touches.length === 1) {
            window.touchDown = true;
            let target = event.target;
            let scrollTop = 0;
            let scrollLeft = 0;
            let activeRendererIndex = memoryDataView.getUint32(instance.exports.ACTIVE_RENDERER_INDEX.value, true);
            while (activeRendererIndex !== 1 && target) {
                scrollLeft += target.scrollLeft;
                scrollTop += target.scrollTop;
                target = target.parentElement;
            }
            window.mousePositionXThisFrame = event.changedTouches[0].pageX + scrollLeft;
            window.mousePositionYThisFrame = event.changedTouches[0].pageY + scrollTop;
        }
    }

    document.addEventListener("touchstart", handleTouch);
    document.addEventListener("touchmove", handleTouch);
    document.addEventListener("touchend", () => {
        window.touchDown = false;
        window.mousePositionXThisFrame = 0;
        window.mousePositionYThisFrame = 0;
    });

    // Wheel event
    let zeroTimeout = null;
    document.addEventListener("wheel", (event) => {
        window.mouseWheelXThisFrame = event.deltaX * -0.1;
        window.mouseWheelYThisFrame = event.deltaY * -0.1;
        clearTimeout(zeroTimeout);
        zeroTimeout = setTimeout(() => {
            window.mouseWheelXThisFrame = 0;
            window.mouseWheelYThisFrame = 0;
        }, 10);
    });

    // Keydown event
    document.addEventListener("keydown", (event) => {
        if (event.key === "ArrowDown") {
            window.arrowKeyDownPressedThisFrame = true;
        }
        if (event.key === "ArrowUp") {
            window.arrowKeyUpPressedThisFrame = true;
        }
        if (event.key === "d") {
            window.dKeyPressedThisFrame = true;
        }
    });

    // Load fonts
    const fontsById = ['Quicksand', 'Calistoga', 'Quicksand', 'Quicksand', 'Quicksand'];
    return Promise.all(fontsById.map(font => document.fonts.load(`12px "${font}"`)));
}