document.addEventListener('DOMContentLoaded', () => {
    fetch('/list')
        .then(response => response.json())
        .then(files => {
            const fileList = document.getElementById('file-list');
            files.forEach(file => {
                const img = document.createElement('img');
                img.src = file;
                img.alt = file;
                img.style.width = '100px';
                img.style.height = '100px';
                fileList.appendChild(img);
            });
        });
});