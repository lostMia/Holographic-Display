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
const maxUploadSize = 1024 * 1024;
const imageSize = 22;

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
  const files = event.dataTransfer.files;

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

  // uploadRawImage(files[0]);
});

function uploadRawImage(file) {
  const formData = new FormData();

  if (file.size > maxUploadSize) {
    alert(`File Size too big! Maximum is ${maxUploadSize/1024}kB`)
  }

  formData.append('file', file);

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/upload', true);

  xhr.upload.onprogress = (event) => {
    if (event.lengthComputable) {
      const percentComplete = (event.loaded / event.total) * 100;
      progressBar.value = percentComplete;
    }
  };

  xhr.onload = () => {
    if (xhr.status === 200) {
      previewImage.src = `/datadump/${file.name}`;
      imagePreviewContainer.style.display = "inline";
      imagePreviewSeparator.style.display = "inline";
    } else {
      alert('Error uploading file.');
    }
  };

  xhr.send(formData);
}

// - - - - - - - - - - - - Experimental Image Conversion - - - - - - - - - - - - //

/*
This is the Json format that will be used.
Using this will open up the possibility of using videos or GIF's further in the future.

{
"frames":            // collection of frames
[
  {
    "delay":"",      // delay in miliseconds between this frame and the one before it.
    "data": [
      123, 255, 244, // first pixel
      0, 255, 255,   // second pixel
      ...,
    ]
  }
]
}
*/
async function handleImageFile(file) {
  const canvas = document.createElement('canvas');
  const img = new Image();
  
  img.src = URL.createObjectURL(file);
  await img.decode();

  // Set canvas size (e.g., 128x128)
  canvas.width = imageSize;
  canvas.height = imageSize;

  const ctx = canvas.getContext('2d');
  ctx.drawImage(img, 0, 0, imageSize, imageSize);

  // Extract pixel data
  const imageData = ctx.getImageData(0, 0, imageSize, imageSize);
  const { data } = imageData;
  const jsonStructure = { frames: [{ delay: 100, data: [] }] };

  for (let i = 0; i < data.length; i += 4) {
    const r = data[i];
    const g = data[i + 1];
    const b = data[i + 2];
    jsonStructure.frames[0].data.push(r, g, b);
  }

  // Convert JSON object to Blob
  const jsonBlob = new Blob([JSON.stringify(jsonStructure)], { type: 'application/json' });

  console.log(jsonStructure);
  console.log(JSON.stringify(jsonStructure))
  
  // Send JSON to ESP32
  await uploadJSON(jsonBlob, 'image.json');
}

async function handleGIFFile(gifFile) {
    const gifReader = new GIFDecoder();
    const frames = [];

    // Read the GIF file
    const arrayBuffer = await gifFile.arrayBuffer();
    gifReader.parse(arrayBuffer);

    // Loop through each frame in the GIF
    for (let i = 0; i < gifReader.frames.length; i++) {
        const frame = gifReader.frames[i];
        const delay = frame.delay; // Delay in milliseconds

        // Create a canvas to extract pixel data
        const canvas = document.createElement('canvas');
        canvas.width = frame.width;
        canvas.height = frame.height;
        const ctx = canvas.getContext('2d');

        // Draw the frame onto the canvas
        ctx.putImageData(new ImageData(new Uint8ClampedArray(frame.data), frame.width, frame.height), 0, 0);

        // Get pixel data from the canvas
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const pixelData = Array.from(imageData.data);

        // Push frame data into frames array
        frames.push({
            delay: delay,
            data: pixelData
        });
    }

    // Return the final JSON structure
    return {
        frames: frames
    };
}

// async function handleGIFFile(file) {
//     const gif = new SuperGif({ gif: document.createElement('img') });
//
//     gif.load(URL.createObjectURL(file));
//
//     await new Promise((resolve) => gif.load(resolve));
//
//     const frameCount = gif.get_length();
//     const size = 22; // Resize target
//
//     const jsonStructure = { frames: [] };
//
//     for (let i = 0; i < frameCount; i++) {
//       gif.move_to(i);
//       const canvas = gif.get_canvas();
//       const ctx = canvas.getContext('2d');
//       const resizedCanvas = document.createElement('canvas');
//       resizedCanvas.width = size;
//       resizedCanvas.height = size;
//       const resizedCtx = resizedCanvas.getContext('2d');
//       resizedCtx.drawImage(canvas, 0, 0, size, size);
//
//       const { data } = resizedCtx.getImageData(0, 0, size, size);
//       const frameData = { delay: gif.get_delay(i), data: [] };
//
//       for (let j = 0; j < data.length; j += 4) {
//         const r = data[j];
//         const g = data[j + 1];
//         const b = data[j + 2];
//         frameData.data.push(r, g, b);
//       }
//
//       jsonStructure.frames.push(frameData);
//     }
//
//     // Convert JSON object to Blob
//     const jsonBlob = new Blob([JSON.stringify(jsonStructure)], { type: 'application/json' });
//
//     console.log(jsonStructure);
//     console.log(JSON.stringify(jsonStructure));
//
//     // Send JSON to ESP32
//     await uploadJSON(jsonBlob, 'animation.json');
//   }
//

async function uploadJSON(jsonBlob, fileName) {
  const formData = new FormData();
  formData.append('file', jsonBlob, fileName);

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/upload', true);

  xhr.upload.onprogress = (event) => {
    if (event.lengthComputable) {
      const percentComplete = (event.loaded / event.total) * 100;
      progressBar.value = percentComplete;
    }
  };

  xhr.onload = () => {
    if (xhr.status === 200) {
      alert('Finished uploading the image! :)');
    } else {
      alert('Error uploading file.');
    }
  };

  xhr.send(formData);
}

// - - - - - - - - - - - - CurrentRPM - - - - - - - - - - - - //

function updateCurrentRPM() {
  fetch('/CurrentRPM')
    .then(response => response.text())
    .then(data => {
      // Update the width of the progress bar based on the value received
      const rpm = parseInt(data);
      document.getElementById('currentRPMLabel').innerText = rpm + " RPM";
    })
    .catch(error => console.error('Error:', error));
}

// setInterval(updateCurrentRPM, 500);

// - - - - - - - - - - - - Data Sending - - - - - - - - - - - - //

let timeout;

// Function to send data to the server
function sendData(input) {
  clearTimeout(timeout); // Clear the previous timeout

  timeout = setTimeout(() => {
    var formData = new FormData();
    formData.append(input.name, input.type === 'checkbox' ? input.checked : input.value);
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

function toggleSection(header) {
  const gridItem = header.parentElement;
  const content = header.nextElementSibling;
  const button = header.querySelector('.toggle-btn');

  content.classList.toggle('collapsed');
  gridItem.classList.toggle('collapsed');
  button.textContent = content.classList.contains('collapsed') ? '+' : '−';
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
