'use strict';

let resolution = {"x": 320, "y": 240};
let width_mm = 48;

function aspectRatio()
{
    return resolution.x / resolution.y;
}

function pixels()
{
    return resolution.x * resolution.y;
}

// Returns a scale ratio of OpenGL units per mm
function getGLscale()
{
    return 2 * aspectRatio() / width_mm * 1000;
}

module.exports = {'resolution': resolution,
                  'aspectRatio': aspectRatio,
                  'pixels': pixels,
                  'getGLscale': getGLscale};
