import {
    CLAY_RENDER_COMMAND_TYPE_NONE,
    CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
    CLAY_RENDER_COMMAND_TYPE_BORDER,
    CLAY_RENDER_COMMAND_TYPE_TEXT,
    CLAY_RENDER_COMMAND_TYPE_IMAGE,
    CLAY_RENDER_COMMAND_TYPE_SCISSOR_START,
    CLAY_RENDER_COMMAND_TYPE_SCISSOR_END,
    CLAY_RENDER_COMMAND_TYPE_CUSTOM,
    GLOBAL_FONT_SCALING_FACTOR,
    fontsById,
    colorDefinition,
    stringDefinition,
    borderDefinition,
    cornerRadiusDefinition,
    rectangleConfigDefinition,
    borderConfigDefinition,
    textConfigDefinition,
    scrollConfigDefinition,
    imageConfigDefinition,
    customConfigDefinition,
    renderCommandDefinition
} from './constants.js';

import { MemoryIsDifferent, readStructAtAddress } from './utils.js';
export function renderLoopHTML(memoryDataView, scratchSpaceAddress, renderCommandSize, htmlRoot, elementCache, textDecoder, instance) {
    let capacity = memoryDataView.getUint32(scratchSpaceAddress, true);
    let length = memoryDataView.getUint32(scratchSpaceAddress + 4, true);
    let arrayOffset = memoryDataView.getUint32(scratchSpaceAddress + 8, true);
    let scissorStack = [{ nextAllocation: { x: 0, y: 0 }, element: htmlRoot, nextElementIndex: 0 }];
    let previousId = 0;

    for (let i = 0; i < length; i++, arrayOffset += renderCommandSize) {
        let entireRenderCommandMemory = new Uint32Array(memoryDataView.buffer.slice(arrayOffset, arrayOffset + renderCommandSize));
        let renderCommand = readStructAtAddress(arrayOffset, renderCommandDefinition);
        let parentElement = scissorStack[scissorStack.length - 1];
        let element = null;
        let isMultiConfigElement = previousId === renderCommand.id.value;

        if (!elementCache[renderCommand.id.value]) {
            let elementType = 'div';
            switch (renderCommand.commandType.value) {
                case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                    if (readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition).link.length.value > 0) {
                        elementType = 'a';
                    }
                    break;
                }
                case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                    elementType = 'img'; break;
                }
                default: break;
            }
            element = document.createElement(elementType);
            element.id = renderCommand.id.value;
            if (renderCommand.commandType.value === CLAY_RENDER_COMMAND_TYPE_SCISSOR_START) {
                element.style.overflow = 'hidden';
            }
            elementCache[renderCommand.id.value] = {
                exists: true,
                element: element,
                previousMemoryCommand: new Uint8Array(0),
                previousMemoryConfig: new Uint8Array(0),
                previousMemoryText: new Uint8Array(0)
            };
        }

        let elementData = elementCache[renderCommand.id.value];
        element = elementData.element;
        if (!isMultiConfigElement && Array.prototype.indexOf.call(parentElement.element.children, element) !== parentElement.nextElementIndex) {
            if (parentElement.nextElementIndex === 0) {
                parentElement.element.insertAdjacentElement('afterbegin', element);
            } else {
                parentElement.element.childNodes[Math.min(parentElement.nextElementIndex - 1, parentElement.element.childNodes.length - 1)].insertAdjacentElement('afterend', element);
            }
        }

        elementData.exists = true;
        let dirty = MemoryIsDifferent(elementData.previousMemoryCommand, entireRenderCommandMemory, renderCommandSize) && !isMultiConfigElement;
        if (!isMultiConfigElement) {
            parentElement.nextElementIndex++;
        }

        previousId = renderCommand.id.value;

        elementData.previousMemoryCommand = entireRenderCommandMemory;
        let offsetX = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.x : 0;
        let offsetY = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.y : 0;
        if (dirty) {
            element.style.transform = `translate(${Math.round(renderCommand.boundingBox.x.value - offsetX)}px, ${Math.round(renderCommand.boundingBox.y.value - offsetY)}px)`;
            element.style.width = Math.round(renderCommand.boundingBox.width.value) + 'px';
            element.style.height = Math.round(renderCommand.boundingBox.height.value) + 'px';
        }

        switch (renderCommand.commandType.value) {
            case CLAY_RENDER_COMMAND_TYPE_NONE: {
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                let config = readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition);
                let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));
                let linkContents = config.link.length.value > 0 ? textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.link.chars.value, config.link.chars.value + config.link.length.value))) : 0;

                memoryDataView.setUint32(0, renderCommand.id.value, true);
                if (linkContents.length > 0 && (window.mouseDownThisFrame || window.touchDown) && instance.exports.Clay_PointerOver(0)) {
                    window.location.href = linkContents;
                }

                if (!dirty && !MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
                    break;
                }

                if (linkContents.length > 0) {
                    element.href = linkContents;
                }

                element.style.cursor = 'default';
                if (linkContents.length > 0 || config.cursorPointer.value) {
                    element.style.pointerEvents = 'all';
                    element.style.cursor = 'pointer';
                }

                elementData.previousMemoryConfig = configMemory;
                let color = config.color;
                element.style.backgroundColor = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;

                if (config.cornerRadius.topLeft.value > 0) {
                    element.style.borderTopLeftRadius = config.cornerRadius.topLeft.value + 'px';
                }
                if (config.cornerRadius.topRight.value > 0) {
                    element.style.borderTopRightRadius = config.cornerRadius.topRight.value + 'px';
                }
                if (config.cornerRadius.bottomLeft.value > 0) {
                    element.style.borderBottomLeftRadius = config.cornerRadius.bottomLeft.value + 'px';
                }
                if (config.cornerRadius.bottomRight.value > 0) {
                    element.style.borderBottomRightRadius = config.cornerRadius.bottomRight.value + 'px';
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                let config = readStructAtAddress(renderCommand.config.value, borderConfigDefinition);
                let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));

                if (!dirty && !MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
                    break;
                }

                elementData.previousMemoryConfig = configMemory;

                if (config.left.width.value > 0) {
                    let color = config.left.color;
                    element.style.borderLeft = `${config.left.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                }
                if (config.right.width.value > 0) {
                    let color = config.right.color;
                    element.style.borderRight = `${config.right.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                }
                if (config.top.width.value > 0) {
                    let color = config.top.color;
                    element.style.borderTop = `${config.top.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                }
                if (config.bottom.width.value > 0) {
                    let color = config.bottom.color;
                    element.style.borderBottom = `${config.bottom.width.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                }

                if (config.cornerRadius.topLeft.value > 0) {
                    element.style.borderTopLeftRadius = config.cornerRadius.topLeft.value + 'px';
                }
                if (config.cornerRadius.topRight.value > 0) {
                    element.style.borderTopRightRadius = config.cornerRadius.topRight.value + 'px';
                }
                if (config.cornerRadius.bottomLeft.value > 0) {
                    element.style.borderBottomLeftRadius = config.cornerRadius.bottomLeft.value + 'px';
                }
                if (config.cornerRadius.bottomRight.value > 0) {
                    element.style.borderBottomRightRadius = config.cornerRadius.bottomRight.value + 'px';
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                let config = readStructAtAddress(renderCommand.config.value, textConfigDefinition);
                let configMemory = new Uint8Array(memoryDataView.buffer.slice(renderCommand.config.value, renderCommand.config.value + config.__size));
                let textContents = renderCommand.text;
                let stringContents = new Uint8Array(memoryDataView.buffer.slice(textContents.chars.value, textContents.chars.value + textContents.length.value));

                if (MemoryIsDifferent(configMemory, elementData.previousMemoryConfig, config.__size)) {
                    element.className = 'text';
                    let textColor = config.textColor;
                    let fontSize = Math.round(config.fontSize.value * GLOBAL_FONT_SCALING_FACTOR);
                    element.style.color = `rgba(${textColor.r.value}, ${textColor.g.value}, ${textColor.b.value}, ${textColor.a.value})`;
                    element.style.fontFamily = fontsById[config.fontId.value];
                    element.style.fontSize = fontSize + 'px';
                    element.style.pointerEvents = config.disablePointerEvents.value ? 'none' : 'all';
                    element.style.userSelect = config.disablePointerEvents.value ? 'none' : 'all';
                    elementData.previousMemoryConfig = configMemory;
                }

                if (stringContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(stringContents, elementData.previousMemoryText, stringContents.length)) {
                    element.innerHTML = textDecoder.decode(stringContents);
                }

                elementData.previousMemoryText = stringContents;
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                scissorStack.push({ nextAllocation: { x: renderCommand.boundingBox.x.value, y: renderCommand.boundingBox.y.value }, element, nextElementIndex: 0 });
                let config = readStructAtAddress(renderCommand.config.value, scrollConfigDefinition);

                if (config.horizontal.value) {
                    element.style.overflowX = 'scroll';
                    element.style.pointerEvents = 'auto';
                }
                if (config.vertical.value) {
                    element.style.overflowY = 'scroll';
                    element.style.pointerEvents = 'auto';
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                scissorStack.splice(scissorStack.length - 1, 1);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                let config = readStructAtAddress(renderCommand.config.value, imageConfigDefinition);
                let srcContents = new Uint8Array(memoryDataView.buffer.slice(config.sourceURL.chars.value, config.sourceURL.chars.value + config.sourceURL.length.value));

                if (srcContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(srcContents, elementData.previousMemoryText, srcContents.length)) {
                    element.src = textDecoder.decode(srcContents);
                }

                elementData.previousMemoryText = srcContents;
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                break;
            }
        }
    }

    for (const key of Object.keys(elementCache)) {
        if (elementCache[key].exists) {
            elementCache[key].exists = false;
        } else {
            elementCache[key].element.remove();
            delete elementCache[key];
        }
    }
}