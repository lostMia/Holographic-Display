const dropZone = document.getElementById('drop_zone');
const progressBar = document.getElementById('progressBar');
const maxUploadSize = 1024 * 1024;

// Synchronize all slider and input field pairs
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

dropZone.addEventListener('dragover', (event) => {
    event.preventDefault();
    event.stopPropagation();
    dropZone.style.borderColor = '#0087F7';
});

dropZone.addEventListener('dragleave', (event) => {
    event.preventDefault();
    event.stopPropagation();
    dropZone.style.borderColor = '#dddddd';
});

dropZone.addEventListener('drop', (event) => {
    event.preventDefault();
    event.stopPropagation();
    dropZone.style.borderColor = '#dddddd';
    const files = event.dataTransfer.files;
    if (files.length > 0) {
        fileInput.files = files;
        handleFiles(files);
    }
});

function handleFiles(files) {
    const file = files[0];
    const formData = new FormData();
    formData.append('file', file);

    if (file.size > maxUploadSize) {
      alert("File too big!");
      return;
    }

    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/upload', true);
    xhr.upload.onprogress = (event) => {
        if (event.lengthComputable) {
            const percentComplete = (event.loaded / event.total) * 100;
            progressBar.value = percentComplete;
        }
    };
    xhr.send(formData);
}

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

function toggleSection(header) {
  const gridItem = header.parentElement;
  const content = header.nextElementSibling;
  const button = header.querySelector('.toggle-btn');

  content.classList.toggle('collapsed');
  gridItem.classList.toggle('collapsed');
  button.textContent = content.classList.contains('collapsed') ? '+' : '−';
}



// document.querySelectorAll('.item-container').forEach(container => {
//     const box = container.querySelector('.item');
//
//     container.addEventListener('mousemove', function (event) {
//         const boxRect = box.getBoundingClientRect();
//         const centerX = boxRect.left + boxRect.width / 2;
//         const centerY = boxRect.top + boxRect.height / 2;
//         const offsetX = (event.clientX - centerX) / boxRect.width * 2;
//         const offsetY = (event.clientY - centerY) / boxRect.height * 2;
//
//         const rotateX = offsetY * 15;
//         const rotateY = offsetX * -15;
//         const translateZ = 30; // Adjust the 30 value for more or less pop out
//
//         box.style.transform = `rotateX(${rotateX}deg) rotateY(${rotateY}deg) translateZ(${translateZ}px)`;
//     });
//
//     container.addEventListener('mouseleave', function () {
//         box.style.transform = `rotateX(0deg) rotateY(0deg) translateZ(0)`;
//     });
// });
