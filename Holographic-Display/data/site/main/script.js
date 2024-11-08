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
    if (files.length > 0) {
        handleFiles(files);
    }
});

function handleFiles(files) {
  const formData = new FormData();

  if (files.length === 0) {
    alert('Please select a file to upload.');
    return;
  }
  
  const file = files[0];

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

setInterval(updateCurrentRPM, 500);

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

// - - - - - - - - - - - - Gradient - - - - - - - - - - - - //

// const follower = document.getElementById('gradient');
//
// document.addEventListener('mousemove', (e) => {
//   follower.style.transform = `translate(${e.pageX}px, ${e.pageY}px)`;
// });

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
