'use strict';

let fs = require('filesaver.js');
let JSZip = require('jszip');

let viewport = require('./viewport.js');
let printer = require('./printer.js');
let ui = require('./ui.js');

////////////////////////////////////////////////////////////////////////////////

// Create a 2D canvas to store our rendered image
let canvas = document.createElement('canvas');
canvas.width = printer.resolution.x;
canvas.height = printer.resolution.y;
let context = canvas.getContext('2d',{colorSpace:'srgb'});

let zip = null;
let slices = null;

function fillCanvasBackgroundWithColor(context, color, width, height) {
    // Get the 2D drawing context from the provided canvas.
//    const context = canvas.getContext('2d');
  
    // We're going to modify the context state, so it's
    // good practice to save the current state first.
//    context.save();
  
    // Normally when you draw on a canvas, the new drawing
    // covers up any previous drawing it overlaps. This is
    // because the default `globalCompositeOperation` is
    // 'source-over'. By changing this to 'destination-over',
    // our new drawing goes behind the existing drawing. This
    // is desirable so we can fill the background, while leaving
    // the chart and any other existing drawing intact.
    // Learn more about `globalCompositeOperation` here:
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/globalCompositeOperation
    context.globalCompositeOperation = 'destination-over';
  
    // Fill in the background. We do this by drawing a rectangle
    // filling the entire canvas, using the provided color.
    context.fillStyle = color;
    context.fillRect(0, 0, width, height);
  
    // Restore the original context state from `context.save()`
    context.restore();
  }

function next(i, n)
{
    if (i < n)
    {
        let data = viewport.getSliceAt(i / n);

        // Copy the pixels to a 2D canvas
        let image = context.createImageData(
                printer.resolution.x, printer.resolution.y,{colorSpace:'srgb'});
        image.data.set(data);

        // Load data into the context
        context.putImageData(image, 0, 0);

        /*context.globalCompositeOperation = 'destination-over';
        context.fillStyle = 'black';
        context.fillRect(0, 0, printer.resolution.x, printer.resolution.y);
        context.restore();*/

        // Convert data to a DataURL and save to the zip file
        let png = canvas.toDataURL('image/bmp');
        let index = i + "";
        while (index.length < 4) index = "0" + index;
        slices.file("out" + index + ".bmp",
                    png.slice(png.indexOf(',') + 1, -1),
                    {base64: true});

        if (i == n - 1)
        {
            ui.setStatus("Saving .zip file...");
        }
        requestAnimationFrame(function() { next(i + 1, n); });
    }
    else
    {
        let content = zip.generate({type: 'blob', compression: 'DEFLATE'});
        let zipName = document.getElementById("filename").value
        fs.saveAs(content, zipName);
        ui.setStatus("");
        ui.enableButtons();
    }
}

// Assign callback to the "slices" button
document.getElementById("slice").onclick = function(event)
{
    if (!viewport.hasModel())
    {
        ui.setStatus("No model loaded!");
        return;
    }

    ui.disableButtons();

    let microns = document.getElementById("height").value;
    let bounds = viewport.getBounds();

    // We map 3 inches to +/-1 on the X axis, so we use that ratio
    // to convert to Z in inches
    let zrange_mm = (bounds.zmax - bounds.zmin) / printer.getGLscale();
    let count = Math.ceil(zrange_mm * 1000 * 1000 / microns);

    zip = new JSZip();
    ui.setStatus("Slicing...");
    slices = zip.folder("slices");
    next(0, count);
}
