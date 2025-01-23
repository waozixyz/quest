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


import { readStructAtAddress } from './utils.js';
export function renderLoopCanvas(memoryDataView, scratchSpaceAddress, renderCommandSize, canvasRoot, canvasContext, textDecoder, instance, imageCache) {
    let capacity = memoryDataView.getUint32(scratchSpaceAddress, true);
    let length = memoryDataView.getUint32(scratchSpaceAddress + 4, true);
    let arrayOffset = memoryDataView.getUint32(scratchSpaceAddress + 8, true);
    canvasRoot.width = window.innerWidth * window.devicePixelRatio;
    canvasRoot.height = window.innerHeight * window.devicePixelRatio;
    canvasRoot.style.width = window.innerWidth + 'px';
    canvasRoot.style.height = window.innerHeight + 'px';
    let ctx = canvasContext;
    let scale = window.devicePixelRatio;

    for (let i = 0; i < length; i++, arrayOffset += renderCommandSize) {
        let renderCommand = readStructAtAddress(arrayOffset, renderCommandDefinition);
        let boundingBox = renderCommand.boundingBox;

        
        switch (renderCommand.commandType.value) {
            case CLAY_RENDER_COMMAND_TYPE_NONE: {
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                let config = readStructAtAddress(renderCommand.config.value, rectangleConfigDefinition);
                let color = config.color;

                ctx.beginPath();
                ctx.fillStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                ctx.roundRect(
                    boundingBox.x.value * scale,
                    boundingBox.y.value * scale,
                    boundingBox.width.value * scale,
                    boundingBox.height.value * scale,
                    [config.cornerRadius.topLeft.value * scale, config.cornerRadius.topRight.value * scale, config.cornerRadius.bottomRight.value * scale, config.cornerRadius.bottomLeft.value * scale]
                );
                ctx.fill();
                ctx.closePath();

                let linkContents = config.link.length.value > 0 ? textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.link.chars.value, config.link.chars.value + config.link.length.value))) : 0;
                memoryDataView.setUint32(0, renderCommand.id.value, true);

                if (linkContents.length > 0 && (window.mouseDownThisFrame || window.touchDown) && instance.exports.Clay_PointerOver(0)) {
                    window.location.href = linkContents;
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                let config = readStructAtAddress(renderCommand.config.value, borderConfigDefinition);
                ctx.beginPath();

                // Top Left Corner
                if (config.cornerRadius.topLeft.value > 0) {
                    let lineWidth = config.top.width.value;
                    let halfLineWidth = lineWidth / 2;
                    let color = config.top.color;

                    ctx.moveTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.arcTo(
                        (boundingBox.x.value + halfLineWidth) * scale,
                        (boundingBox.y.value + halfLineWidth) * scale,
                        (boundingBox.x.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale,
                        (boundingBox.y.value + halfLineWidth) * scale,
                        config.cornerRadius.topLeft.value * scale
                    );
                    ctx.stroke();
                }

                // Top border
                if (config.top.width.value > 0) {
                    let lineWidth = config.top.width.value;
                    let halfLineWidth = lineWidth / 2;
                    let color = config.top.color;

                    ctx.moveTo((boundingBox.x.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
                    ctx.lineTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.topRight.value - halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.stroke();
                }

                // Top Right Corner
                if (config.cornerRadius.topRight.value > 0) {
                    let lineWidth = config.top.width.value;
                    let halfLineWidth = lineWidth / 2;
                    let color = config.top.color;

                    ctx.moveTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.topRight.value - halfLineWidth) * scale, (boundingBox.y.value + halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.arcTo(
                        (boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale,
                        (boundingBox.y.value + halfLineWidth) * scale,
                        (boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale,
                        (boundingBox.y.value + config.cornerRadius.topRight.value + halfLineWidth) * scale,
                        config.cornerRadius.topRight.value * scale
                    );
                    ctx.stroke();
                }

                // Right border
                if (config.right.width.value > 0) {
                    let color = config.right.color;
                    let lineWidth = config.right.width.value;
                    let halfLineWidth = lineWidth / 2;

                    ctx.moveTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topRight.value + halfLineWidth) * scale);
                    ctx.lineTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.stroke();
                }

                // Bottom Right Corner
                if (config.cornerRadius.bottomRight.value > 0) {
                    let color = config.bottom.color;
                    let lineWidth = config.bottom.width.value;
                    let halfLineWidth = lineWidth / 2;

                    ctx.moveTo((boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.arcTo(
                        (boundingBox.x.value + boundingBox.width.value - halfLineWidth) * scale,
                        (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale,
                        (boundingBox.x.value + boundingBox.width.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale,
                        (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale,
                        config.cornerRadius.bottomRight.value * scale
                    );
                    ctx.stroke();
                }

                // Bottom Border
                if (config.bottom.width.value > 0) {
                    let color = config.bottom.color;
                    let lineWidth = config.bottom.width.value;
                    let halfLineWidth = lineWidth / 2;

                    ctx.moveTo((boundingBox.x.value + config.cornerRadius.bottomLeft.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
                    ctx.lineTo((boundingBox.x.value + boundingBox.width.value - config.cornerRadius.bottomRight.value - halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.stroke();
                }

                // Bottom Left Corner
                if (config.cornerRadius.bottomLeft.value > 0) {
                    let color = config.bottom.color;
                    let lineWidth = config.bottom.width.value;
                    let halfLineWidth = lineWidth / 2;

                    ctx.moveTo((boundingBox.x.value + config.cornerRadius.bottomLeft.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.arcTo(
                        (boundingBox.x.value + halfLineWidth) * scale,
                        (boundingBox.y.value + boundingBox.height.value - halfLineWidth) * scale,
                        (boundingBox.x.value + halfLineWidth) * scale,
                        (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomLeft.value - halfLineWidth) * scale,
                        config.cornerRadius.bottomLeft.value * scale
                    );
                    ctx.stroke();
                }

                // Left Border
                if (config.left.width.value > 0) {
                    let color = config.left.color;
                    let lineWidth = config.left.width.value;
                    let halfLineWidth = lineWidth / 2;

                    ctx.moveTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + boundingBox.height.value - config.cornerRadius.bottomLeft.value - halfLineWidth) * scale);
                    ctx.lineTo((boundingBox.x.value + halfLineWidth) * scale, (boundingBox.y.value + config.cornerRadius.topLeft.value + halfLineWidth) * scale);
                    ctx.lineWidth = lineWidth * scale;
                    ctx.strokeStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                    ctx.stroke();
                }

                ctx.closePath();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                let config = readStructAtAddress(renderCommand.config.value, textConfigDefinition);
                let textContents = renderCommand.text;
                let stringContents = new Uint8Array(memoryDataView.buffer.slice(textContents.chars.value, textContents.chars.value + textContents.length.value));
                let fontSize = config.fontSize.value * GLOBAL_FONT_SCALING_FACTOR * scale;

                ctx.font = `${fontSize}px ${fontsById[config.fontId.value]}`;
                let color = config.textColor;
                ctx.textBaseline = 'middle';
                ctx.fillStyle = `rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`;
                ctx.fillText(textDecoder.decode(stringContents), boundingBox.x.value * scale, (boundingBox.y.value + boundingBox.height.value / 2 + 1) * scale);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                ctx.save();
                ctx.beginPath();
                ctx.rect(boundingBox.x.value * scale, boundingBox.y.value * scale, boundingBox.width.value * scale, boundingBox.height.value * scale);
                ctx.clip();
                ctx.closePath();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                ctx.restore();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                let config = readStructAtAddress(renderCommand.config.value, imageConfigDefinition);
                let src = textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(config.sourceURL.chars.value, config.sourceURL.chars.value + config.sourceURL.length.value)));

                if (!imageCache[src]) {
                    imageCache[src] = {
                        image: new Image(),
                        loaded: false,
                    };
                    imageCache[src].image.onload = () => imageCache[src].loaded = true;
                    imageCache[src].image.src = src;
                } else if (imageCache[src].loaded) {
                    ctx.drawImage(imageCache[src].image, boundingBox.x.value * scale, boundingBox.y.value * scale, boundingBox.width.value * scale, boundingBox.height.value * scale);
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                break;
            }
        }
    }
}
