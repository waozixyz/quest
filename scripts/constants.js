// scripts/constants.js

export const CLAY_RENDER_COMMAND_TYPE_NONE = 0;
export const CLAY_RENDER_COMMAND_TYPE_RECTANGLE = 1;
export const CLAY_RENDER_COMMAND_TYPE_BORDER = 2;
export const CLAY_RENDER_COMMAND_TYPE_TEXT = 3;
export const CLAY_RENDER_COMMAND_TYPE_IMAGE = 4;
export const CLAY_RENDER_COMMAND_TYPE_SCISSOR_START = 5;
export const CLAY_RENDER_COMMAND_TYPE_SCISSOR_END = 6;
export const CLAY_RENDER_COMMAND_TYPE_CUSTOM = 7;
export const GLOBAL_FONT_SCALING_FACTOR = 0.8;

export const fontsById = [
    'Quicksand',
    'Calistoga',
    'Quicksand',
    'Quicksand',
    'Quicksand',
];

export const colorDefinition = { type: 'struct', members: [
    {name: 'r', type: 'float' },
    {name: 'g', type: 'float' },
    {name: 'b', type: 'float' },
    {name: 'a', type: 'float' },
]};

export const stringDefinition = { type: 'struct', members: [
    {name: 'length', type: 'uint32_t' },
    {name: 'chars', type: 'uint32_t' },
]};

export const borderDefinition = { type: 'struct', members: [
    {name: 'width', type: 'uint32_t'},
    {name: 'color', ...colorDefinition},
]};

export const cornerRadiusDefinition = { type: 'struct', members: [
    {name: 'topLeft', type: 'float'},
    {name: 'topRight', type: 'float'},
    {name: 'bottomLeft', type: 'float'},
    {name: 'bottomRight', type: 'float'},
]};

export const rectangleConfigDefinition = { name: 'rectangle', type: 'struct', members: [
    { name: 'color', ...colorDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition },
    { name: 'link', ...stringDefinition },
    { name: 'cursorPointer', type: 'uint8_t' },
]};

export const borderConfigDefinition = { name: 'text', type: 'struct', members: [
    { name: 'left', ...borderDefinition },
    { name: 'right', ...borderDefinition },
    { name: 'top', ...borderDefinition },
    { name: 'bottom', ...borderDefinition },
    { name: 'betweenChildren', ...borderDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition }
]};

export const textConfigDefinition = { name: 'text', type: 'struct', members: [
   { name: 'textColor', ...colorDefinition },
   { name: 'fontId', type: 'uint16_t' },
   { name: 'fontSize', type: 'uint16_t' },
   { name: 'letterSpacing', type: 'uint16_t' },
   { name: 'lineSpacing', type: 'uint16_t' },
   { name: 'wrapMode', type: 'uint32_t' },
   { name: 'disablePointerEvents', type: 'uint8_t' }
]};

export const scrollConfigDefinition = { name: 'text', type: 'struct', members: [
    { name: 'horizontal', type: 'bool' },
    { name: 'vertical', type: 'bool' },
]};

export const imageConfigDefinition = { name: 'image', type: 'struct', members: [
    { name: 'imageData', type: 'uint32_t' },
    { name: 'sourceDimensions', type: 'struct', members: [
        { name: 'width', type: 'float' },
        { name: 'height', type: 'float' },
    ]},
    { name: 'sourceURL', ...stringDefinition }
]};

export const customConfigDefinition = { name: 'custom', type: 'struct', members: [
    { name: 'customData', type: 'uint32_t' },
]};

export const renderCommandDefinition = {
    name: 'CLay_RenderCommand',
    type: 'struct',
    members: [
        { name: 'boundingBox', type: 'struct', members: [
            { name: 'x', type: 'float' },
            { name: 'y', type: 'float' },
            { name: 'width', type: 'float' },
            { name: 'height', type: 'float' },
        ]},
        { name: 'config', type: 'uint32_t'},
        { name: 'text', ...stringDefinition },
        { name: 'id', type: 'uint32_t' },
        { name: 'commandType', type: 'uint32_t', },
    ]
};