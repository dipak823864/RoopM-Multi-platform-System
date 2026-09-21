
console.log("[ROOPM MASTER] All Phase 1 & 2 Subsystems Live & Operational...");

// 1. Live Uptime Clock
let seconds = 0;
setInterval(function() {
    seconds++;
    let timer = document.getElementById("live-timer");
    if (timer) timer.innerText = "⚡ Uptime: " + seconds + "s";
}, 1000);

// 2. Play / Pause Track Toggle
let isPlaying = true;
let playBtn = document.getElementById("btn-play");
if (playBtn) {
    playBtn.addEventListener("click", function() {
        isPlaying = !isPlaying;
        playBtn.innerText = isPlaying ? "⏸ Pause Track" : "▶ Play Track";
    });
}

// 3. Live 60 FPS HTML5 <canvas> 2D Audio Spectrum Visualizer
let canvas = document.getElementById("waveform-canvas");
if (canvas) {
    let ctx = canvas.getContext("2d");
    let waveStep = 0;
    
    setInterval(function() {
        if (!ctx) return;
        waveStep++;
        
        ctx.clearRect(0, 0, 480, 90);
        
        let barCount = 32;
        let barWidth = 11;
        let spacing = 4;
        
        for (let i = 0; i < barCount; i++) {
            let freq = Math.sin((i * 0.4) + (waveStep * 0.15)) * Math.cos((i * 0.2) + (waveStep * 0.1));
            let height = 15 + Math.abs(freq) * 65;
            let x = i * (barWidth + spacing) + 8;
            let y = 90 - height;
            
            if (i % 3 === 0) ctx.fillStyle = "#38bdf8";      // Sky Blue
            else if (i % 3 === 1) ctx.fillStyle = "#10b981"; // Emerald
            else ctx.fillStyle = "#a855f7";                  // Purple
            
            ctx.fillRect(x, y, barWidth, height);
        }
    }, 33);
}

// 4. 🌟 Testing element.innerHTML Live Injection
let injectBtn = document.getElementById("btn-inject-html");
if (injectBtn) {
    injectBtn.addEventListener("click", function() {
        let container = document.getElementById("injected-container");
        if (container) {
            container.innerHTML = "<p class='toast-msg'>✨ Success: Live element.innerHTML parsed and rendered by C++ Lexbor Engine!</p>";
            console.log("[JS DOM] Live element.innerHTML injected successfully!");
        }
    });
}

// 5. 🌟 Testing document.querySelectorAll() & classList.toggle() on Like Buttons
let likeButtons = document.querySelectorAll(".like-btn");
for (let i = 0; i < likeButtons.length; i++) {
    likeButtons[i].addEventListener("click", function() {
        likeButtons[i].classList.toggle("liked");
        console.log("[JS CLASSLIST] Toggled 'liked' class on button #" + i);
    });
}

// 6. Dynamic Track Creator: document.createElement & appendChild
let addBtn = document.getElementById("btn-add-track");
let trackCount = 2;

if (addBtn) {
    addBtn.addEventListener("click", function() {
        let titleInput = document.getElementById("input-title");
        let artistInput = document.getElementById("input-artist");
        let playlist = document.getElementById("playlist-container");

        let songTitle = (titleInput && titleInput.value !== "") ? titleInput.value : "Neon Midnight Echoes";
        let artistGenre = (artistInput && artistInput.value !== "") ? artistInput.value : "Cyber Synthwave • 132 BPM";

        trackCount++;

        let newRow = document.createElement("div");
        newRow.className = "track-row";

        let newMeta = document.createElement("div");
        newMeta.className = "track-meta";

        let newH4 = document.createElement("h4");
        newH4.innerText = (trackCount < 10 ? "0" : "") + trackCount + ". " + songTitle;

        let newP = document.createElement("p");
        newP.innerText = artistGenre;

        newMeta.appendChild(newH4);
        newMeta.appendChild(newP);

        let newBitrate = document.createElement("div");
        newBitrate.className = "track-bitrate";
        newBitrate.innerText = "Hi-Res Master";

        let newBtn = document.createElement("button");
        newBtn.className = "like-btn grad-emerald";
        newBtn.innerText = "❤️ Like";

        newBtn.addEventListener("click", function() {
            newBtn.classList.toggle("liked");
        });

        newRow.appendChild(newMeta);
        newRow.appendChild(newBitrate);
        newRow.appendChild(newBtn);

        if (playlist) {
            playlist.appendChild(newRow);
        }

        if (titleInput) titleInput.value = "";
        if (artistInput) artistInput.value = "";
    });
}
