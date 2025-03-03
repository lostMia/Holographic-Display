import { parseGIF, decompressFrames } from 'gifuct-js'


// document.onreadystatechange = function () {
//   if (document.readyState !== "complete") {
//     document.body.style.visibility = "hidden";
//   } else {
//     document.body.style.visibility = "visible";
//   }
// };

// - - - - - - - - - - - - Constants - - - - - - - - - - - - //

const maxUploadSize = 1024 * 1024 * 8;
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
const previewImage = document.getElementById('previewImage');
const imagePreviewContainer = document.getElementById('imagePreviewContainer');
const imagePreviewSeparator = document.getElementById('imagePreviewSeparator');
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


window.processGIFFrame = async function processGIFFrame(frame) {
  const canvas = document.createElement('canvas');
  canvas.width = frame.dims.width;
  canvas.height = frame.dims.height;
  const ctx = canvas.getContext('2d');

  const imageData = ctx.createImageData(frame.dims.width, frame.dims.height);
  const colorTable = frame.colorTable;

  for (let i = 0; i < frame.pixels.length; i++) {
    const pixelIndex = frame.pixels[i];
    if (pixelIndex !== frame.transparentIndex) {
      const [r, g, b] = colorTable[pixelIndex];
      imageData.data[i * 4] = r;
      imageData.data[i * 4 + 1] = g;
      imageData.data[i * 4 + 2] = b;
      imageData.data[i * 4 + 3] = 255;
    }
  }

  ctx.putImageData(imageData, 0, 0);
  ctx.drawImage(canvas, 0, 0, imageSize, imageSize);

  const resizedImageData = ctx.getImageData(0, 0, imageSize, imageSize);
  const { data } = resizedImageData;
  const frameData = new Uint8Array(imageSize * imageSize * 3);
  let index = 0;

  for (let i = 0; i < data.length; i += 4) {
    frameData[index++] = data[i];     // Red
    frameData[index++] = data[i + 1]; // Green
    frameData[index++] = data[i + 2]; // Blue
  }

  return { delay: frame.delay, data: frameData };
}

window.extractFramesFromGIF = async function extractFramesFromGIF(file) {
  const buffer = await file.arrayBuffer();
  const gif = parseGIF(buffer);
  return decompressFrames(gif, true);
} 


window.uploadBinary = async function uploadBinary(binaryBlob, fileName) {
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
    .catch(error => 
    {
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
    xhr.onreadystatechange = function () {
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
const yData  = []

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

// - - - - - - - - - - - - Parallax effect - - - - - - - - - - - - //

document.querySelectorAll('#imagePreviewContainer').forEach(container => {
  const box = container.querySelector('.previewImage');

  box.style.transition = 'transform 0.4s ease-out';

  container.addEventListener('mousemove', function (event) {
    const boxRect = box.getBoundingClientRect();
    const centerX = boxRect.left + boxRect.width / 2;
    const centerY = boxRect.top + boxRect.height / 2;
    const offsetX = (event.clientX - centerX) / boxRect.width * 2;
    const offsetY = (event.clientY - centerY) / boxRect.height * 2;

    const rotateX = offsetY * 15;
    const rotateY = offsetX * -15;
    const translateZ = 30; // Adjust the 30 value for more or less pop out

    box.style.transform = `rotateX(${rotateX}deg) rotateY(${rotateY}deg) translateZ(${translateZ}px)`;
  });

  container.addEventListener('mouseleave', function () {
    setTimeout(() => {
      box.style.transform = `rotateX(0deg) rotateY(0deg) translateZ(0)`;
    }, 1000);
  });
});
