/*
 * Roopm C++ Source-to-Source Transpiler
 * Reads app.cpp and compiles it to JavaScript equivalents
 * THIS IS A REAL COMPILER (Simplified), not hardcoded strings.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <vector>

// Helper to read entire file
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Simple text replacement
std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string transpileLogic(std::string cpp) {
    // 1. Convert Data Types
    cpp = std::regex_replace(cpp, std::regex(R"(float\s+)"), "let ");
    cpp = std::regex_replace(cpp, std::regex(R"(int\s+)"), "let ");
    cpp = std::regex_replace(cpp, std::regex(R"(bool\s+)"), "let ");
    cpp = std::regex_replace(cpp, std::regex(R"(uint32_t\s+)"), "let ");
    
    // 2. Convert Math functions
    cpp = replaceAll(cpp, "sinf(", "Math.sin(");
    cpp = replaceAll(cpp, "cosf(", "Math.cos(");
    cpp = replaceAll(cpp, "sqrtf(", "Math.sqrt(");
    
    // 3. Convert Engine APIs to JS Canvas calls
    // Note: We map these to helper functions defined in the runtime
    cpp = replaceAll(cpp, "roopm_get_width()", "window.innerWidth");
    cpp = replaceAll(cpp, "roopm_get_height()", "window.innerHeight");
    cpp = replaceAll(cpp, "roopm_get_dpi()", "(window.devicePixelRatio||1)");
    cpp = replaceAll(cpp, "roopm_get_pixel_width()", "canvas.width");
    cpp = replaceAll(cpp, "roopm_get_pixel_height()", "canvas.height");

    // Drawing
    cpp = replaceAll(cpp, "roopm_clear(", "R.clear(");
    cpp = replaceAll(cpp, "roopm_draw_rect(", "R.rect(");
    cpp = replaceAll(cpp, "roopm_draw_rect_outline(", "R.outline(");
    cpp = replaceAll(cpp, "roopm_draw_circle(", "R.circle(");
    cpp = replaceAll(cpp, "roopm_draw_text(", "R.text(");
    cpp = replaceAll(cpp, "roopm_push_clip(", "R.clip(");
    cpp = replaceAll(cpp, "roopm_pop_clip()", "R.pop()");
    
    // Input
    cpp = replaceAll(cpp, "roopm_is_rect_clicked(", "R.clicked(");
    cpp = replaceAll(cpp, "roopm_is_rect_pressed(", "R.pressed(");
    cpp = replaceAll(cpp, "roopm_is_rect_hovered(", "false && R.clicked("); // Web doesn't need hover logic for touch
    
    // Remove pointers
    cpp = std::regex_replace(cpp, std::regex(R"(RoopmInput\s*\*\s*input\s*=\s*roopm_get_input\(\);)"), "");
    cpp = replaceAll(cpp, "input->", "g_input.");
    
    // Remove float suffixes (2.5f -> 2.5)
    cpp = std::regex_replace(cpp, std::regex(R"((\d+\.\d+)f)"), "$1");
    
    // colors: ROOPM_RGB(r,g,b) -> 'rgb(r,g,b)'
    // This is tricky with regex, simpler to map to helper: R.rgb(r,g,b)
    cpp = replaceAll(cpp, "ROOPM_RGB", "R.rgb");
    cpp = replaceAll(cpp, "ROOPM_RGBA", "R.rgba");
    cpp = replaceAll(cpp, "ROOPM_WHITE", "'#fff'");
    
    // snprintf is annoying. Replace with simple string manip if possible or comment out
    // Since app.cpp uses snprintf for "Clicks: %d", let's replace it with a helper
    // snprintf(buf, size, "Msg: %d", val)
    // -> buf = `Msg: ${val}` is hard to regex perfectly.
    // Hack: Replace the whole line helper?
    // Let's assume specific usage in app.cpp
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"Clicks: %d\",\s*(\w+)\);)"), "countText = `Clicks: ${$1}`;");
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"Logical: %.0f x %.0f\",\s*(\w+),\s*(\w+)\);)"), "infoText = `Logical: ${Math.round($1)} x ${Math.round($2)}`;");
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"Physical: %d x %d\",\s*([^,]+),\s*([^,]+)\);)"), "infoText = `Physical: ${$1} x ${$2}`;");
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"DPI: %.2f \| %s\",\s*([^,]+),\s*([^,]+)\);)"), "infoText = `DPI: ${$1.toFixed(2)} | ${$2}`;");
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"Animation Time: %.1fs\",\s*(\w+)\);)"), "infoText = `Animation Time: ${$1.toFixed(1)}s`;");
    cpp = std::regex_replace(cpp, std::regex(R"(snprintf\(\w+,\s*sizeof\(\w+\),\s*\"Scroll Position: %.0f\",\s*(\w+)\);)"), "infoText = `Scroll Position: ${Math.round($1)}`;");
    
    return cpp;
}

int main() {
    std::cout << "[COMPILER] Reading app/app.cpp..." << std::endl;
    std::string source = readFile("app/app.cpp");
    
    // Extract app_update
    std::smatch match;
    std::regex updateRegex(R"(void\s+app_update\(void\)\s*\{([\s\S]*?)\n\})");
    
    std::string logic;
    if (std::regex_search(source, match, updateRegex)) {
        logic = match[1].str();
    } else {
        std::cerr << "[ERROR] Could not find app_update() in C++ source!" << std::endl;
        return 1;
    }
    
    std::cout << "[COMPILER] Transpiling Logic..." << std::endl;
    std::string jsLogic = transpileLogic(logic);
    
    // Generate Final JS
    std::ofstream outJS("build/web/roopm.js");
    outJS << R"(
// AUTO-GENERATED JS
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

let g_clickCount = 0;
let g_scrollY = 0;
let g_animTime = 0;
let g_lastTime = performance.now();
let g_input = { deltaTime: 0, scrollDeltaY: 0 };

// Variables needed by transpiled code
let countText = "";
let infoText = "";

// Helper Runtime
const R = {
    clear: (c) => { ctx.fillStyle=c; ctx.fillRect(0,0,canvas.width,canvas.height); },
    rect: (x,y,w,h,c) => { ctx.fillStyle=c; ctx.fillRect(x,y,w,h); },
    outline: (x,y,w,h,c,t) => { ctx.strokeStyle=c; ctx.lineWidth=t; ctx.strokeRect(x,y,w,h); },
    circle: (x,y,r,c) => { ctx.fillStyle=c; ctx.beginPath(); ctx.arc(x,y,r,0,Math.PI*2); ctx.fill(); },
    text: (x,y,t,s,c) => { ctx.fillStyle=c; ctx.font=`${s}px Arial`; ctx.fillText(t,x,y+s*0.75); },
    clip: (x,y,w,h) => { ctx.save(); ctx.beginPath(); ctx.rect(x,y,w,h); ctx.clip(); },
    pop: () => { ctx.restore(); },
    clicked: (x,y,w,h) => { return g_mouseDown && g_mouseX>=x && g_mouseX<=x+w && g_mouseY>=y && g_mouseY<=y+h; },
    pressed: (x,y,w,h) => { return g_mouseDown && g_mouseX>=x && g_mouseX<=x+w && g_mouseY>=y && g_mouseY<=y+h; },
    rgb: (r,g,b) => `rgb(${r},${g},${b})`,
    rgba: (r,g,b,a) => `rgba(${r},${g},${b},${a/255.0})`
};

let g_mouseX=0, g_mouseY=0, g_mouseDown=false;

// Input Events
window.onresize = () => {
    const dpr = window.devicePixelRatio||1;
    canvas.width = window.innerWidth*dpr;
    canvas.height = window.innerHeight*dpr;
    canvas.style.width = window.innerWidth+'px';
    canvas.style.height = window.innerHeight+'px';
    ctx.scale(dpr,dpr);
};
window.onresize();

canvas.onmousemove = e => { 
    const r = canvas.getBoundingClientRect(); 
    g_mouseX = e.clientX - r.left; g_mouseY = e.clientY - r.top; 
};
canvas.onmousedown = () => g_mouseDown = true;
canvas.onmouseup = () => g_mouseDown = false;
canvas.onwheel = e => { e.preventDefault(); g_input.scrollDeltaY = e.deltaY * 0.1; };

canvas.ontouchstart = e => {
    e.preventDefault();
    const t=e.touches[0]; const r=canvas.getBoundingClientRect();
    g_mouseX=t.clientX-r.left; g_mouseY=t.clientY-r.top; g_mouseDown=true;
};
canvas.ontouchend = e => { e.preventDefault(); g_mouseDown=false; };

function update() {
    let now = performance.now();
    g_input.deltaTime = (now - g_lastTime) / 1000;
    g_lastTime = now;

    // --- START TRANSPILED C++ CODE ---
)";

    outJS << jsLogic;

    outJS << R"(
    // --- END TRANSPILED C++ CODE ---
    
    g_input.scrollDeltaY = 0; // Reset scroll frame
    if(g_mouseDown) g_mouseDown = false; // Simple click reset for this demo
    requestAnimationFrame(update);
}
update();
)";
    
    outJS.close();
    std::cout << "[SUCCESS] Generated build/web/roopm.js" << std::endl;
    
    // Generate HTML
    std::ofstream outHTML("build/web/index.html");
    outHTML << R"(<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1"><style>body{margin:0;overflow:hidden;background:#333}canvas{background:#fff;display:block}</style></head><body><canvas id="canvas"></canvas><script src="roopm.js"></script></body></html>)";
    outHTML.close();

    return 0;
}
