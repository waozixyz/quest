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

import { readStructAtAddress, getStructTotalSize, getTextDimensions } from './utils.js';

let memoryDataView = null;
let instance = null;
let scratchSpaceAddress = 8;
let heapSpaceAddress = 0;
let renderCommandSize = 0;



export function getMemoryDataView() {
    if (!memoryDataView) {
        throw new Error("memoryDataView is not initialized. Call initializeWasm() first.");
    }
    return memoryDataView;
}

export function getInstance() {
    if (!instance) {
        throw new Error("instance is not initialized. Call initializeWasm() first.");
    }
    return instance;
}
export function getScratchSpaceAddress() {
    return scratchSpaceAddress;
}

export function getHeapSpaceAddress() {
    return heapSpaceAddress;
}

export function getRenderCommandSize() {
    return renderCommandSize;
}

export async function initializeWasm() {
    var printCharBuffers = [null,[],[]];

    var out = (text) => { console.log(text); };
    var err = (text) => { console.error(text); };

    var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead = NaN) => {
        var endIdx = idx + maxBytesToRead;
        var endPtr = idx;
        while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;

        if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
            return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
        }
        var str = '';
        while (idx < endPtr) {
            var u0 = heapOrArray[idx++];
            if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
            var u1 = heapOrArray[idx++] & 63;
            if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
            var u2 = heapOrArray[idx++] & 63;
            if ((u0 & 0xF0) == 0xE0) {
                u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
            } else {
                u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
            }
            if (u0 < 0x10000) {
                str += String.fromCharCode(u0);
            } else {
                var ch = u0 - 0x10000;
                str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
            }
        }
        return str;
    };


    var printChar = (stream, curr) => {
        var buffer = printCharBuffers[stream];
        if (!buffer) {
            console.error("No buffer for stream:", stream); // Debug
            return;
        }
        if (curr === 0 || curr === 10) {
            let output = UTF8ArrayToString(buffer);
            (stream === 1 ? out : err)(output);
            buffer.length = 0;
        } else {
            buffer.push(curr);
        }
    };
    

    const importObject = {
        
        env: {
            // Add this function to convert a tm struct to a Unix timestamp
            _mktime_js: (tmPtr) => {
                // Read the tm struct from memory
                const tm_sec = memoryDataView.getInt32(tmPtr, true);       // Seconds (0-59)
                const tm_min = memoryDataView.getInt32(tmPtr + 4, true);   // Minutes (0-59)
                const tm_hour = memoryDataView.getInt32(tmPtr + 8, true);  // Hours (0-23)
                const tm_mday = memoryDataView.getInt32(tmPtr + 12, true); // Day of the month (1-31)
                const tm_mon = memoryDataView.getInt32(tmPtr + 16, true);  // Month (0-11, 0 = January)
                const tm_year = memoryDataView.getInt32(tmPtr + 20, true); // Year (years since 1900)
                const tm_wday = memoryDataView.getInt32(tmPtr + 24, true); // Day of the week (0-6, 0 = Sunday)
                const tm_yday = memoryDataView.getInt32(tmPtr + 28, true); // Day of the year (0-365)
                const tm_isdst = memoryDataView.getInt32(tmPtr + 32, true); // Daylight saving time flag

                // Convert the tm struct to a JavaScript Date object
                const date = new Date(
                    tm_year + 1900, // Year (add 1900 to get the full year)
                    tm_mon,         // Month (0-11)
                    tm_mday,        // Day of the month (1-31)
                    tm_hour,        // Hours (0-23)
                    tm_min,         // Minutes (0-59)
                    tm_sec          // Seconds (0-59)
                );

                // Convert the Date object to a Unix timestamp (seconds since the epoch)
                const timestamp = Math.floor(date.getTime() / 1000);

                return timestamp; // Return the Unix timestamp
            },
            // Add this function to handle heap resizing
            emscripten_resize_heap: (requestedSize) => {
                console.log(`Resizing heap to ${requestedSize} bytes`);
                const oldMemory = importObject.env.memory;
                const oldBuffer = oldMemory.buffer;

                // Calculate the new size (round up to the nearest 64KB page)
                const PAGE_SIZE = 65536; // 64KB
                const newSize = Math.ceil(requestedSize / PAGE_SIZE) * PAGE_SIZE;

                // Create a new memory instance with the new size
                const newMemory = new WebAssembly.Memory({
                    initial: Math.ceil(newSize / PAGE_SIZE),
                    maximum: oldMemory.maximum,
                });

                // Copy the old memory contents to the new memory
                const newBuffer = newMemory.buffer;
                new Uint8Array(newBuffer).set(new Uint8Array(oldBuffer));

                // Replace the old memory with the new memory
                importObject.env.memory = newMemory;
                memoryDataView = new DataView(newBuffer);

                // Notify Emscripten that the memory has grown
                if (importObject.env.emscripten_notify_memory_growth) {
                    importObject.env.emscripten_notify_memory_growth(0); // 0 is the memory index
                }

                return 1; // Return 1 to indicate success
            },
            _tzset_js: () => {
                console.log("_tzset_js called (no-op)");
            },
            _emscripten_memcpy_js: (dest, src, num) => {
                const destArray = new Uint8Array(importObject.env.memory.buffer, dest, num);
                const srcArray = new Uint8Array(importObject.env.memory.buffer, src, num);
                destArray.set(srcArray);
                return dest;
            },
            emscripten_date_now: () => Date.now(),
            memory: new WebAssembly.Memory({ initial: 256, maximum: 512 }),
            table: new WebAssembly.Table({ initial: 0, element: 'anyfunc' }),
            emscripten_asm_const_int: (x) => x,
            emscripten_notify_memory_growth: (memoryIndex) => {
                const buffer = importObject.env.memory.buffer;
                memoryDataView = new DataView(buffer);
                console.log(`Memory grew, updated memory view for memory index: ${memoryIndex}`);
            },
            printf: msg => console.log(msg),
            __localtime_js: (time_low, time_high, tmPtr) => {
                const time = (time_low + (time_high * 2 ** 32)) / 1000; // Convert to seconds
                const date = new Date(time * 1000);

                memoryDataView.setInt32(tmPtr, date.getSeconds(), true);
                memoryDataView.setInt32(tmPtr + 4, date.getMinutes(), true);
                memoryDataView.setInt32(tmPtr + 8, date.getHours(), true);
                memoryDataView.setInt32(tmPtr + 12, date.getDate(), true);
                memoryDataView.setInt32(tmPtr + 16, date.getMonth(), true);
                memoryDataView.setInt32(tmPtr + 20, date.getFullYear() - 1900, true);
                memoryDataView.setInt32(tmPtr + 24, date.getDay(), true);

                const yday = Math.floor((date - new Date(date.getFullYear(), 0, 1)) / (24 * 60 * 60 * 1000));
                memoryDataView.setInt32(tmPtr + 28, yday, true);

                const timezoneOffset = -date.getTimezoneOffset() * 60;
                memoryDataView.setInt32(tmPtr + 36, timezoneOffset, true);

                const isDst = new Date(date.getFullYear(), 6, 1).getTimezoneOffset() !== timezoneOffset;
                memoryDataView.setInt32(tmPtr + 32, isDst ? 1 : 0, true);
            },
            JS_SaveHabits: (collectionPtr) => {
                try {
                    const memory = new DataView(instance.exports.memory.buffer);
                    const HABIT_SIZE = 12056;
                    const HABIT_DAY_SIZE = 12;
                    const MAX_HABITS = 10;

                    const habitsCount = memory.getUint32(collectionPtr + (HABIT_SIZE * MAX_HABITS), true);
                    const activeHabitId = memory.getUint32(collectionPtr + (HABIT_SIZE * MAX_HABITS) + 4, true);
                    
                    const habitsData = {
                        habits: [],
                        active_habit_id: activeHabitId
                    };

                    for (let i = 0; i < habitsCount; i++) {
                        const habitOffset = collectionPtr + (i * HABIT_SIZE);
                        
                        // Read name
                        let name = "";
                        for (let j = 0; j < 32; j++) {
                            const char = memory.getUint8(habitOffset + j);
                            if (char === 0) break;
                            name += String.fromCharCode(char);
                        }

                        const id = memory.getUint32(habitOffset + 32, true);
                        const colorOffset = habitOffset + 36;
                        const daysOffset = habitOffset + 52;
                        const daysCount = memory.getUint32(habitOffset + 12052, true);

                        const habit = {
                            id,
                            name,
                            color: {
                                r: memory.getFloat32(colorOffset, true),
                                g: memory.getFloat32(colorOffset + 4, true),
                                b: memory.getFloat32(colorOffset + 8, true),
                                a: memory.getFloat32(colorOffset + 12, true)
                            },
                            calendar_days: []
                        };

                        for (let j = 0; j < daysCount; j++) {
                            const dayOffset = daysOffset + (j * HABIT_DAY_SIZE);
                            habit.calendar_days.push({
                                date: memory.getUint32(dayOffset, true),
                                day_index: memory.getUint32(dayOffset + 4, true),
                                completed: memory.getUint8(dayOffset + 8) !== 0
                            });
                        }

                        habitsData.habits.push(habit);
                    }

                    localStorage.setItem("habitsData", JSON.stringify(habitsData));
                } catch (error) {
                    console.error("Error in JS_SaveHabits:", error);
                    console.error(error.stack);
                }
            },
            deleteHabitFunction: (collectionPtr, habit_id) => {
                try {
                    const stored = localStorage.getItem("habitsData");
                    if (!stored) return;

                    let habitsData = JSON.parse(stored);
                    
                    // Find and remove the habit
                    const habitIndex = habitsData.habits.findIndex(h => h.id === habit_id);
                    if (habitIndex === -1) return;
                    
                    habitsData.habits.splice(habitIndex, 1);
                    
                    // Update remaining habit IDs to match their array positions
                    habitsData.habits.forEach((habit, index) => {
                        habit.id = index;
                    });
                    
                    // Update active_habit_id if needed
                    if (habitsData.active_habit_id === habit_id) {
                        if (habitsData.habits.length > 0) {
                            if (habitIndex > 0) {
                                habitsData.active_habit_id = habitIndex - 1;
                            } else {
                                habitsData.active_habit_id = 0;
                            }
                        }
                    } else if (habitsData.active_habit_id > habit_id) {
                        habitsData.active_habit_id--;
                    }
                    
                    localStorage.setItem("habitsData", JSON.stringify(habitsData));
                } catch (error) {
                    console.error("Error in deleteHabitFunction:", error);
                }
            },
            addNewHabitFunction: (collectionPtr) => {
                try {
                    const stored = localStorage.getItem("habitsData");
                    let habitsData = stored ? JSON.parse(stored) : { habits: [], active_habit_id: 0 };
                    
                    const newHabitId = habitsData.habits.length;
                    
                    // Create new habit with default values
                    const newHabit = {
                        id: newHabitId,
                        name: `Habit ${newHabitId + 1}`,
                        color: {
                            r: 148,  // Default primary color values
                            g: 47,
                            b: 74,
                            a: 255
                        },
                        calendar_days: []
                    };
                    
                    habitsData.habits.push(newHabit);
                    habitsData.active_habit_id = newHabitId;
                    
                    localStorage.setItem("habitsData", JSON.stringify(habitsData));
                } catch (error) {
                    console.error("Error in addNewHabitFunction:", error);
                }
            },
            JS_SaveTodos: (collectionPtr) => {
                try {
                    const memory = new DataView(instance.exports.memory.buffer);
                    const TODO_SIZE = 1052; // Assuming similar structure to habits
                    const MAX_TODOS = 50;

                    const todosCount = memory.getUint32(collectionPtr + (TODO_SIZE * MAX_TODOS), true);
                    
                    const todosData = {
                        todos: [],
                        active_day: ""
                    };

                    // Read active day
                    const activeDayOffset = collectionPtr + (TODO_SIZE * MAX_TODOS) + 4;
                    let activeDay = "";
                    for (let j = 0; j < 10; j++) {
                        const char = memory.getUint8(activeDayOffset + j);
                        if (char === 0) break;
                        activeDay += String.fromCharCode(char);
                    }
                    todosData.active_day = activeDay;

                    for (let i = 0; i < todosCount; i++) {
                        const todoOffset = collectionPtr + (i * TODO_SIZE);
                        
                        // Read text
                        let text = "";
                        for (let j = 0; j < 256; j++) {
                            const char = memory.getUint8(todoOffset + j);
                            if (char === 0) break;
                            text += String.fromCharCode(char);
                        }

                        const id = memory.getUint32(todoOffset + 256, true);
                        const position = memory.getUint32(todoOffset + 260, true);
                        const completed = memory.getUint8(todoOffset + 264) !== 0;
                        const createdAt = Number(memory.getBigUint64(todoOffset + 268, true));
                        
                        // Read day
                        let day = "";
                        for (let j = 0; j < 10; j++) {
                            const char = memory.getUint8(todoOffset + 276 + j);
                            if (char === 0) break;
                            day += String.fromCharCode(char);
                        }

                        const todo = {
                            id,
                            text,
                            position,
                            completed,
                            created_at: createdAt,
                            day
                        };

                        todosData.todos.push(todo);
                    }

                    localStorage.setItem("todosData", JSON.stringify(todosData));
                } catch (error) {
                    console.error("Error in JS_SaveTodos:", error);
                    console.error(error.stack);
                }
            },

            JS_LoadTodos: (collectionPtr) => {
                try {
                    const stored = localStorage.getItem("todosData");
                    if (!stored) {
                        console.log("No stored todos data found");
                        return;
                    }

                    const todosData = JSON.parse(stored);
                    const memory = new DataView(instance.exports.memory.buffer);
                    const TODO_SIZE = 1052;
                    const MAX_TODOS = 50;

                    // Write todos count
                    memory.setUint32(collectionPtr + (TODO_SIZE * MAX_TODOS), todosData.todos.length, true);

                    // Write active day
                    const activeDayOffset = collectionPtr + (TODO_SIZE * MAX_TODOS) + 4;
                    const encoder = new TextEncoder();
                    const activeDayBytes = encoder.encode(todosData.active_day);
                    for (let j = 0; j < 10; j++) {
                        memory.setUint8(activeDayOffset + j, j < activeDayBytes.length ? activeDayBytes[j] : 0);
                    }

                    // Write each todo
                    for (let i = 0; i < todosData.todos.length; i++) {
                        const todo = todosData.todos[i];
                        const todoOffset = collectionPtr + (i * TODO_SIZE);

                        // Write text
                        const textBytes = encoder.encode(todo.text);
                        for (let j = 0; j < 256; j++) {
                            memory.setUint8(todoOffset + j, j < textBytes.length ? textBytes[j] : 0);
                        }

                        // Write id
                        memory.setUint32(todoOffset + 256, todo.id, true);

                        // Write position
                        memory.setUint32(todoOffset + 260, todo.position, true);

                        // Write completed
                        memory.setUint8(todoOffset + 264, todo.completed ? 1 : 0);

                        // Write created_at
                        memory.setBigUint64(todoOffset + 268, BigInt(todo.created_at), true);

                        // Write day
                        const dayBytes = encoder.encode(todo.day);
                        for (let j = 0; j < 10; j++) {
                            memory.setUint8(todoOffset + 276 + j, j < dayBytes.length ? dayBytes[j] : 0);
                        }
                    }
                } catch (error) {
                    console.error("Error in JS_LoadTodos:", error);
                    console.error(error.stack);
                }
            },
            JS_LoadHabits: (collectionPtr) => {
                try {
                    const stored = localStorage.getItem("habitsData");
                    if (!stored) {
                        console.log("No stored habits data found");
                        return;
                    }

                    const habitsData = JSON.parse(stored);
                    const memory = new DataView(instance.exports.memory.buffer);
                    const HABIT_SIZE = 12056;
                    const HABIT_DAY_SIZE = 12;
                    const MAX_HABITS = 10;

                    // Write habits count and active habit id
                    memory.setUint32(collectionPtr + (HABIT_SIZE * MAX_HABITS), habitsData.habits.length, true);
                    memory.setUint32(collectionPtr + (HABIT_SIZE * MAX_HABITS) + 4, habitsData.active_habit_id, true);


                    // Write each habit
                    for (let i = 0; i < habitsData.habits.length; i++) {
                        const habit = habitsData.habits[i];
                        const habitOffset = collectionPtr + (i * HABIT_SIZE);

                        // Write name
                        const encoder = new TextEncoder();
                        const nameBytes = encoder.encode(habit.name);
                        for (let j = 0; j < 32; j++) {
                            memory.setUint8(habitOffset + j, j < nameBytes.length ? nameBytes[j] : 0);
                        }

                        // Write id
                        memory.setUint32(habitOffset + 32, habit.id, true);

                        // Write color
                        const colorOffset = habitOffset + 36;
                        memory.setFloat32(colorOffset, habit.color.r, true);
                        memory.setFloat32(colorOffset + 4, habit.color.g, true);
                        memory.setFloat32(colorOffset + 8, habit.color.b, true);
                        memory.setFloat32(colorOffset + 12, habit.color.a, true);

                        // Write calendar days
                        const daysOffset = habitOffset + 52;
                        for (let j = 0; j < habit.calendar_days.length; j++) {
                            const day = habit.calendar_days[j];
                            const dayOffset = daysOffset + (j * HABIT_DAY_SIZE);
                            memory.setUint32(dayOffset, day.date, true);
                            memory.setUint32(dayOffset + 4, day.day_index, true);
                            memory.setUint8(dayOffset + 8, day.completed ? 1 : 0);
                        }

                        // Write days count
                        memory.setUint32(habitOffset + 12052, habit.calendar_days.length, true);
                    }
                } catch (error) {
                    console.error("Error in JS_LoadHabits:", error);
                    console.error(error.stack);
                }
            },
            JS_AddTodo: (collectionPtr, textPtr) => {
                try {
                    const stored = localStorage.getItem("todosData");
                    let todosData = stored ? JSON.parse(stored) : { todos: [], active_day: "Monday" };
                    
                    // Read the text from memory
                    let text = "";
                    const textDecoder = new TextDecoder();
                    for (let i = 0; i < 256; i++) {  // MAX_TODO_TEXT
                        const char = memoryDataView.getUint8(textPtr + i);
                        if (char === 0) break;
                        text += String.fromCharCode(char);
                    }
                    
                    const newTodo = {
                        id: todosData.todos.length,
                        text: text,
                        position: todosData.todos.length,
                        completed: false,
                        created_at: Date.now(),
                        day: todosData.active_day
                    };
                    
                    todosData.todos.push(newTodo);
                    localStorage.setItem("todosData", JSON.stringify(todosData));
                } catch (error) {
                    console.error("Error in addTodoFunction:", error);
                }
            },

            JS_DeleteTodo: (collectionPtr, todoId) => {
                try {
                    const stored = localStorage.getItem("todosData");
                    if (!stored) return;

                    let todosData = JSON.parse(stored);
                    todosData.todos = todosData.todos.filter(todo => todo.id !== todoId);
                    
                    // Update remaining todo IDs and positions
                    todosData.todos.forEach((todo, index) => {
                        todo.id = index;
                        todo.position = index;
                    });
                    
                    localStorage.setItem("todosData", JSON.stringify(todosData));
                } catch (error) {
                    console.error("Error in deleteTodoFunction:", error);
                }
            },

            JS_ToggleTodo: (collectionPtr, todoId) => {
                try {
                    const stored = localStorage.getItem("todosData");
                    if (!stored) return;

                    let todosData = JSON.parse(stored);
                    const todo = todosData.todos.find(t => t.id === todoId);
                    if (todo) {
                        todo.completed = !todo.completed;
                        localStorage.setItem("todosData", JSON.stringify(todosData));
                    }
                } catch (error) {
                    console.error("Error in toggleTodoFunction:", error);
                }
            },

            JS_SetActiveDay: (collectionPtr, dayPtr) => {
                try {
                    const stored = localStorage.getItem("todosData");
                    let todosData = stored ? JSON.parse(stored) : { todos: [], active_day: "Monday" };
                    
                    // Read the day from memory
                    let day = "";
                    for (let i = 0; i < 10; i++) {
                        const char = memoryDataView.getUint8(dayPtr + i);
                        if (char === 0) break;
                        day += String.fromCharCode(char);
                    }
                    
                    todosData.active_day = day;
                    localStorage.setItem("todosData", JSON.stringify(todosData));
                } catch (error) {
                    console.error("Error in setActiveDayFunction:", error);
                }
            },
        },
        wasi_snapshot_preview1: {
            proc_exit: () => {},
            fd_write: (fd, iov, iovcnt, pnum) => {
                var num = 0;
                for (var i = 0; i < iovcnt; i++) {
                    var ptr = memoryDataView.getUint32(iov + i*8, true);
                    var len = memoryDataView.getUint32(iov + i*8 + 4, true);
                    for (var j = 0; j < len; j++) {
                        let byte = memoryDataView.getUint8(ptr + j);
                        printChar(fd, byte);
                    }
                    num += len;
                }
                memoryDataView.setUint32(pnum, num, true);
                return 0;
            },
            fd_close: () => 0,
            fd_seek: () => 0,
            fd_read: () => 0,
            environ_sizes_get: () => 0,
            environ_get: () => 0,
            clock_time_get: (id, precision, timePtr) => {
                const nowNs = BigInt(Date.now()) * BigInt(1e6);
                memoryDataView.setBigUint64(timePtr, nowNs, true);
                return 0; // Success
            },

            random_get: (bufferPtr, bufferLen) => {
                const buffer = new Uint8Array(importObject.env.memory.buffer, bufferPtr, bufferLen);
                crypto.getRandomValues(buffer);
                return 0;
            }
        },
        clay: {
            measureTextFunction: (addressOfDimensions, textToMeasure, addressOfConfig) => {
                let stringLength = memoryDataView.getUint32(textToMeasure, true);
                let pointerToString = memoryDataView.getUint32(textToMeasure + 4, true);
                let textConfig = readStructAtAddress(addressOfConfig, textConfigDefinition);
                let textDecoder = new TextDecoder("utf-8");
                let text = textDecoder.decode(memoryDataView.buffer.slice(pointerToString, pointerToString + stringLength));
                let sourceDimensions = getTextDimensions(text, `${Math.round(textConfig.fontSize.value * GLOBAL_FONT_SCALING_FACTOR)}px ${fontsById[textConfig.fontId.value]}`);
                memoryDataView.setFloat32(addressOfDimensions, sourceDimensions.width, true);
                memoryDataView.setFloat32(addressOfDimensions + 4, sourceDimensions.height, true);
            },
            queryScrollOffsetFunction: (addressOfOffset, elementId) => {
                let container = document.getElementById(elementId.toString());
                if (container) {
                    memoryDataView.setFloat32(addressOfOffset, -container.scrollLeft, true);
                    memoryDataView.setFloat32(addressOfOffset + 4, -container.scrollTop, true);
                }
            }
        }
    };
    

    const { instance: wasmInstance } = await WebAssembly.instantiateStreaming(fetch("/index.wasm"), importObject);
    instance = wasmInstance;

    // Set up memory and other parameters
    memoryDataView = new DataView(new Uint8Array(instance.exports.memory.buffer).buffer);
    scratchSpaceAddress = instance.exports.__heap_base.value;
    heapSpaceAddress = instance.exports.__heap_base.value + 1024;
    let arenaAddress = scratchSpaceAddress + 8;

    // Initialize memory and pass parameters to the WebAssembly instance
    createMainArena(arenaAddress, heapSpaceAddress);
    memoryDataView.setFloat32(instance.exports.__heap_base.value, window.innerWidth, true);
    memoryDataView.setFloat32(instance.exports.__heap_base.value + 4, window.innerHeight, true);

    // Call initialization function
    instance.exports.Clay_Initialize(arenaAddress, instance.exports.__heap_base.value);

    // Log the exports to see what is available
    console.log(instance.exports);

    // Calculate render command size
    renderCommandSize = getStructTotalSize(renderCommandDefinition);
}

function createMainArena(arenaStructAddress, arenaMemoryAddress) {
    let memorySize = instance.exports.Clay_MinMemorySize();
    instance.exports.Clay_CreateArenaWithCapacityAndMemory(arenaStructAddress, memorySize, arenaMemoryAddress);
}