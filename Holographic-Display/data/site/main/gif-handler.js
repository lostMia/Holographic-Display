import { parseGIF, decompressFrames } from './gifuct-js'

export async function handleGIFFile(file) {
  const buffer = await file.arrayBuffer(); // Read the file as an ArrayBuffer
  const gif = parseGIF(buffer); // Parse the GIF
  const frames = decompressFrames(gif, true); // Decompress all frames

  const size = 128; // Target size for cropping and resizing
  const jsonStructure = { frames: [] };

  // Create a canvas for resizing
  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  canvas.width = size;
  canvas.height = size;

  for (const frame of frames) {
    const imageData = new ImageData(
      new Uint8ClampedArray(frame.patch),
      gif.lsd.width,
      gif.lsd.height
    );

    // Draw the original frame on the canvas
    canvas.width = gif.lsd.width;
    canvas.height = gif.lsd.height;
    ctx.putImageData(imageData, 0, 0);

    // Resize the frame
    const resizedCanvas = document.createElement('canvas');
    resizedCanvas.width = size;
    resizedCanvas.height = size;
    const resizedCtx = resizedCanvas.getContext('2d');
    resizedCtx.drawImage(canvas, 0, 0, size, size);

    // Extract pixel data
    const resizedImageData = resizedCtx.getImageData(0, 0, size, size);
    const { data } = resizedImageData;

    const frameData = { delay: frame.delay, data: [] };
    for (let i = 0; i < data.length; i += 4) {
      const r = data[i];
      const g = data[i + 1];
      const b = data[i + 2];
      frameData.data.push(r, g, b);
    }

    jsonStructure.frames.push(frameData);
  }

  // Convert JSON object to Blob
  const jsonBlob = new Blob([JSON.stringify(jsonStructure)], { type: 'application/json' });

  console.log(jsonStructure);
  console.log(JSON.stringify(jsonStructure));

  // Send JSON to ESP32
  await uploadJSON(jsonBlob, 'animation.json');
}


