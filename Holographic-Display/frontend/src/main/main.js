import { parseGIF, decompressFrames } from 'gifuct-js'
import { Chart, LineController, LineElement, PointElement, LinearScale, CategoryScale, Title } from 'chart.js';

// Register required components to minimize the size impact of chart.js... for some reason. pwp
Chart.register(LineController, LineElement, PointElement, LinearScale, CategoryScale, Title);

// - - - - - - - - - - - - Constants - - - - - - - - - - - - //

const maxUploadSize = 8000000;
const imageSize = 128;

const canvas = document.createElement('canvas');
canvas.width = imageSize;
canvas.height = imageSize;

// - - - - - - - - - - - - Sliders - - - - - - - - - - - - //

document.querySelectorAll('.slider-group').forEach(group => {
  const slider = group.querySelector('.slider');
  const manualInput = group.querySelector('.manualSlider');

  slider.addEventListener('input', function() {
    manualInput.value = this.value;
  });

  manualInput.addEventListener('input', function() {
    // Ensure input value is within slider's range
    const min = slider.min || 0;
    const max = slider.max || 100;
    this.value = Math.max(min, Math.min(max, this.value));
    slider.value = this.value;
  });
});

// - - - - - - - - - - - - Upload Box - - - - - - - - - - - - //

const dropZone = document.getElementById('dropZone');
const progressBar = document.getElementById('progressBar');
const fileInput = document.getElementById('fileInput');


fileInput.addEventListener('change', (event) => {
  handleFiles(event.target.files);
});

dropZone.addEventListener('dragover', (event) => {
  event.preventDefault();
  event.stopPropagation();
  dropZone.style.borderColor = '#ff4060';
});

dropZone.addEventListener('dragleave', (event) => {
  event.preventDefault();
  event.stopPropagation();
  dropZone.style.borderColor = '#555555';
});

dropZone.addEventListener('drop', (event) => {
  event.preventDefault();
  event.stopPropagation();
  dropZone.style.borderColor = '#555557';

  handleFiles(event.dataTransfer.files)
});

window.handleFiles = function handleFiles(files) {
  if (files.length === 0) {
    alert('Please select a file to upload.');
    return;
  }

  const file = files[0];

  // Detect file type
  if (file.type === 'image/gif') {
    console.log('GIF detected.');
    handleGIFFile(file);
  } else if (file.type.startsWith('image/')) {
    console.log('Image detected.');
    handleImageFile(file);
  } else {
    alert('Unsupported file type. Please upload a valid image or GIF.');
  }
}

// - - - - - - - - - - - - Image Upload - - - - - - - - - - - - //

window.handleImageFile = async function handleImageFile(file) {
  const canvas = document.createElement('canvas');
  const img = new Image();

  img.src = URL.createObjectURL(file);
  await img.decode();

  canvas.width = imageSize;
  canvas.height = imageSize;

  // Scale the image.
  const ctx = canvas.getContext('2d');
  ctx.drawImage(img, 0, 0, imageSize, imageSize);

  // Extract pixel data
  const imageData = ctx.getImageData(0, 0, imageSize, imageSize);
  const { data } = imageData;

  // Allocate binary buffer (RGB only, no alpha)
  const binaryData = new Uint8Array(((imageSize * imageSize) * 3) + 2);
  let index = 0;

  // Set the delay to zero
  binaryData[index++] = 0x00;
  binaryData[index++] = 0x00;

  for (let i = 0; i < data.length; i += 4) {
    binaryData[index++] = data[i];     // Red
    binaryData[index++] = data[i + 1]; // Green
    binaryData[index++] = data[i + 2]; // Blue
  }

  // Convert binary data to Blob
  const binaryBlob = new Blob([binaryData], { type: 'application/octet-stream' });

  console.log(binaryData);

  // Send binary data to ESP32
  await uploadBinary(binaryBlob, 'data.bin');
}

// - - - - - - - - - - - - GIF Upload - - - - - - - - - - - - //

window.handleGIFFile = async function handleGIFFile(file) {
  const frames = await extractFramesFromGIF(file);
  const frameCount = frames.length;

  // Allocate binary buffer (RGB data + delay per frame)
  const binaryData = new Uint8Array((imageSize * imageSize * 3 * frameCount) + (frameCount * 2));
  let index = 0;

  for (const frame of frames) {
    const processedFrame = await processGIFFrame(frame);
    binaryData[index++] = processedFrame.delay & 0xff;
    binaryData[index++] = (processedFrame.delay >> 8) & 0xff;

    for (const pixel of processedFrame.data) {
      binaryData[index++] = pixel;
    }
  }

  // Convert binary data to Blob
  const binaryBlob = new Blob([binaryData], { type: 'application/octet-stream' });

  console.log(binaryData);

  // Send binary data to ESP32
  await uploadBinary(binaryBlob, 'data.bin');
}


window.extractFramesFromGIF = async function extractFramesFromGIF(file) {
  const buffer = await file.arrayBuffer();
  const gif = parseGIF(buffer);
  const frames = decompressFrames(gif, true);

  // Create a persistent canvas to reconstruct full frames
  const fullCanvas = document.createElement('canvas');
  fullCanvas.width = gif.lsd.width;
  fullCanvas.height = gif.lsd.height;
  const fullCtx = fullCanvas.getContext('2d');

  // Clear to background color (or transparent and stuff)
  fullCtx.clearRect(0, 0, fullCanvas.width, fullCanvas.height);

  const reconstructedFrames = [];

  for (const frame of frames) {
    const { pixels, dims, delay, colorTable, transparentIndex, disposalType } = frame;

    // Create a temporary canvas for the current frame
    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = fullCanvas.width;
    tempCanvas.height = fullCanvas.height;
    const tempCtx = tempCanvas.getContext('2d');

    // Copy to new canvas
    tempCtx.drawImage(fullCanvas, 0, 0);

    // Resize and get data.
    const imageData = tempCtx.createImageData(dims.width, dims.height);

    for (let i = 0; i < pixels.length; i++) {
      const pixelIndex = pixels[i];
      if (pixelIndex !== transparentIndex) {
        const [r, g, b] = colorTable[pixelIndex];
        imageData.data[i * 4] = r;
        imageData.data[i * 4 + 1] = g;
        imageData.data[i * 4 + 2] = b;
        imageData.data[i * 4 + 3] = 255; // Full opacity
      }
    }

    // Draw the frame onto the temp canvas
    tempCtx.putImageData(imageData, dims.left, dims.top);
    // Copy the result to the fullCanvas
    fullCtx.drawImage(tempCanvas, 0, 0);
    // Store the fully constructed frame
    reconstructedFrames.push({
      delay,
      imageData: fullCtx.getImageData(0, 0, fullCanvas.width, fullCanvas.height)
    });

    if (disposalType === 2) {
      // clear the drawn frame
      fullCtx.clearRect(dims.left, dims.top, dims.width, dims.height);
    } else if (disposalType === 3) {
      // Restore previous: reset to the previous full frame
      // Since this is too much work for too little payoff I'll just... not lmao.
    }
  }

  return reconstructedFrames;
};

// Update processGIFFrame to accept reconstructed frames
window.processGIFFrame = async function processGIFFrame(frame) {
  const canvas = document.createElement('canvas');
  canvas.width = frame.imageData.width;
  canvas.height = frame.imageData.height;
  const ctx = canvas.getContext('2d');

  ctx.putImageData(frame.imageData, 0, 0);

  // Create a resized version
  const resizedCanvas = document.createElement('canvas');
  resizedCanvas.width = imageSize;
  resizedCanvas.height = imageSize;
  const resizedCtx = resizedCanvas.getContext('2d');
  resizedCtx.drawImage(canvas, 0, 0, imageSize, imageSize);

  // Extract resized pixel data
  const resizedImageData = resizedCtx.getImageData(0, 0, imageSize, imageSize);
  const { data } = resizedImageData;
  const frameData = new Uint8Array(imageSize * imageSize * 3);
  let index = 0;

  for (let i = 0; i < data.length; i += 4) {
    frameData[index++] = data[i];     // Red
    frameData[index++] = data[i + 1]; // Green
    frameData[index++] = data[i + 2]; // Blue
  }

  return { delay: frame.delay, data: frameData };
};


window.uploadBinary = async function uploadBinary(binaryBlob, fileName) {
  if (binaryBlob.size > maxUploadSize) {
    alert('File is too large! Maximum frame count is 162!');
    return;
  }

  const formData = new FormData();
  formData.append('file', binaryBlob, fileName);

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/upload', true);

  xhr.upload.onprogress = (event) => {
    if (event.lengthComputable) {
      const percentComplete = (event.loaded / event.total) * 100;
      progressBar.value = percentComplete;
    }
  };

  xhr.onload = () => {
    progressBar.value = 0;

    if (xhr.status === 200) {
      alert('Finished uploading the image! :)');
    } else {
      alert('Error uploading file.');
    }
  };

  xhr.send(formData);
}

// - - - - - - - - - - - - CurrentRPM - - - - - - - - - - - - //

window.updateCurrentRPM = function updateCurrentRPM() {
  fetch('/CurrentRPM')
    .then(response => response.text())
    .then(data => {
      // Update the width of the progress bar based on the value received
      const rpm = parseInt(data);
      addNewRPMValue(rpm)
      document.getElementById('currentRPMLabel').innerText = rpm + " RPM";
    })
    .catch(error => {
      console.error('Error:', error)
      addNewRPMValue(0)
    });
}

setInterval(updateCurrentRPM, 1000);

// - - - - - - - - - - - - Data Sending - - - - - - - - - - - - //

let timeout;

// Function to send data to the server
window.sendData = function sendData(input) {
  clearTimeout(timeout); // Clear the previous timeout

  timeout = setTimeout(() => {
    var formData = new FormData();
    formData.append(input.name, input.type === 'checkbox' ? input.checked : input.value);
    console.log(formData)
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/post", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200) {
        console.log("Data sent and response loaded");
      }
    };
    xhr.send(new URLSearchParams(formData).toString());
  }, 80); // Delay sending the request.
}

// Add event listeners to all form elements
document.querySelectorAll('#dataForm input, #dataForm select').forEach(function(element) {
  element.addEventListener('input', function(event) {
    sendData(event.target);
  });
});

// - - - - - - - - - - - - Item-Toggle - - - - - - - - - - - - //

window.toggleSection = function toggleSection(header) {
  const gridItem = header.parentElement;
  const content = header.nextElementSibling;
  const button = header.querySelector('.toggle-btn');

  content.classList.toggle('collapsed');
  gridItem.classList.toggle('collapsed');
  button.textContent = content.classList.contains('collapsed') ? '+' : '−';
}

// - - - - - - - - - - - - Power Chart - - - - - - - - - - - - //

const rpmChartElement = document.getElementById('RPMChart').getContext('2d');
const xLabels = []
const yData = []

for (let i = -20; i <= 0; i++) {
  xLabels.push(i.toString());
  yData.push(0);
}

const rpmChart = new Chart(rpmChartElement, {
  type: 'line',
  data: {
    labels: xLabels,
    datasets: [{
      label: 'RPM',
      data: yData,
      borderColor: '#ff4060',
      borderWidth: 2,
      fill: false
    }]
  },
  options: {
    scales: {
      x: { title: { display: true, text: 'Time passed (s)' } },
      y: { title: { display: true, text: 'Current RPM' }, beginAtZero: true }
    }
  }
});

window.addNewRPMValue = function addNewRPMValue(value) {
  yData.shift();
  yData.push(value);
  rpmChart.update()
}
