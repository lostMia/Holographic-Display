const dropZone = document.getElementById('drop_zone');
const progressBar = document.getElementById('progressBar');
const maxUploadSize = 1024 * 1024;

// Slider and Manual Input Synchronization
const slider = document.getElementById('slider');
const manualSlider = document.getElementById('manualSlider');

slider.addEventListener('input', function() {
  manualSlider.value = this.value;
});

manualSlider.addEventListener('input', function() {
  slider.value = this.value;
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



document.querySelectorAll('.item-container').forEach(container => {
    const box = container.querySelector('.item');

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
        box.style.transform = `rotateX(0deg) rotateY(0deg) translateZ(0)`;
    });
});
