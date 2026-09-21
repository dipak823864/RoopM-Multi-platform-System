using System;
using System.IO;
using System.Text.RegularExpressions;

class RoopmWebBuilder
{
    static void Main(string[] args)
    {
        Console.WriteLine("==========================================");
        Console.WriteLine("       RoopM Smart Web Compiler v12.5     ");
        Console.WriteLine("==========================================");

        string appCppPath = Path.Combine("app", "app.cpp");
        string webDir = Path.Combine("build", "web");

        if (!File.Exists(appCppPath)) {
            Console.WriteLine("[ERROR] Could not find app/app.cpp");
            Environment.Exit(1);
        }

        string cppCode = File.ReadAllText(appCppPath);
        Console.WriteLine(string.Format("[READ] Processed {0}", appCppPath));

        // 1. Hoist All Global Statics to JS
        string staticVars = "";
        var staticRegex = new Regex(@"static\s+(float|int|bool|uint32_t|char)\s+(\w+)\s*=\s*(.*?);");
        foreach (Match m in staticRegex.Matches(cppCode))
        {
            string type = m.Groups[1].Value;
            string name = m.Groups[2].Value;
            string val = m.Groups[3].Value;
            if (type == "float") val = val.Replace("f", "");
            
            if (!staticVars.Contains("let " + name + " ")) {
                staticVars += string.Format("let {0} = {1};\n", name, val);
            }
        }

        // 2. Extract app_update() logic
        var match = Regex.Match(cppCode, @"void\s+app_update\(void\)\s*\{([\s\S]*?)\n\}");
        if (!match.Success) {
            Console.WriteLine("[ERROR] Could not find app_update() function!");
            Environment.Exit(1);
        }

        string logicBody = match.Groups[1].Value;
        logicBody = staticRegex.Replace(logicBody, "");

        // 3. Transpile logic
        string jsLogic = TranspileLogic(logicBody);

        Directory.CreateDirectory(webDir);
        GenerateStyleCSS(webDir);
        GenerateRoopmJS(webDir, jsLogic, staticVars);
        GenerateIndexHTML(webDir);

        Console.WriteLine(string.Format("[SUCCESS] Web App Generated at: {0}", Path.GetFullPath(webDir)));
    }

    static string TranspileLogic(string cpp)
    {
        // 1. Remove C-Style Casts
        cpp = cpp.Replace("(float)", "");
        cpp = cpp.Replace("(int)", "");

        // 🌟 2. SMART DIRECT ARRAY PARSER
        cpp = Regex.Replace(cpp, @"\b(?:const\s+)?(?:char\s*\*+|uint32_t|int|float|bool)\s+(\w+)\[\d*\]\s*=\s*\{([\s\S]*?)\};", "let $1 = [$2];");
        
        // 🌟 3. Convert string buffers
        cpp = Regex.Replace(cpp, @"\bchar\s+(\w+)\[\d+\];", "let $1 = '';");
        
        // 🌟 4. Convert basic types to 'let'
        cpp = Regex.Replace(cpp, @"\b(uint32_t|int|float|bool|char)\s+", "let ");
        cpp = Regex.Replace(cpp, @"\b(\d+\.\d+)f\b", "$1");

        // 🌟 Safety Filter: Clean any accidental 'let x[128]' remaining in code
        cpp = Regex.Replace(cpp, @"\blet\s+(\w+)\[\d+\];", "let $1 = '';");

        // Math mappings
        cpp = cpp.Replace("sinf(", "Math.sin(");
        cpp = cpp.Replace("cosf(", "Math.cos(");
        cpp = cpp.Replace("sqrtf(", "Math.sqrt(");
        cpp = cpp.Replace("fabsf(", "Math.abs(");
        cpp = cpp.Replace("expf(", "Math.exp(");
        cpp = cpp.Replace("fminf(", "Math.min(");
        cpp = cpp.Replace("fmaxf(", "Math.max(");

        // Engine API mappings
        cpp = cpp.Replace("roopm_get_width()", "window.innerWidth");
        cpp = cpp.Replace("roopm_get_height()", "window.innerHeight");
        cpp = cpp.Replace("roopm_get_dpi()", "(window.devicePixelRatio||1)");
        cpp = cpp.Replace("roopm_get_pixel_width()", "canvas.width");
        cpp = cpp.Replace("roopm_get_pixel_height()", "canvas.height");
        cpp = cpp.Replace("roopm_measure_text(", "R.measureText(");

        // Word Wrapping API
        cpp = cpp.Replace("draw_text_wrapped(", "R.textWrapped(");
        cpp = cpp.Replace("measure_text_wrapped_height(", "R.measureTextWrappedHeight(");

        cpp = cpp.Replace("roopm_clear(", "R.clear(");
        cpp = cpp.Replace("roopm_draw_rect_rounded_gradient(", "R.rectRoundedGradient(");
        cpp = cpp.Replace("roopm_draw_rect_rounded_outline(", "R.outlineRounded(");
        cpp = cpp.Replace("roopm_draw_rect_rounded(", "R.rectRounded(");
        cpp = cpp.Replace("roopm_draw_rect_outline(", "R.outline(");
        cpp = cpp.Replace("roopm_draw_rect(", "R.rect(");
        cpp = cpp.Replace("roopm_draw_circle_outline(", "R.circleOutline(");
        cpp = cpp.Replace("roopm_draw_circle(", "R.circle(");
        cpp = cpp.Replace("roopm_draw_line(", "R.line(");
        cpp = cpp.Replace("roopm_draw_text_centered(", "R.textCentered(");
        cpp = cpp.Replace("roopm_draw_text(", "R.text(");
        cpp = cpp.Replace("roopm_push_clip(", "R.clip(");
        cpp = cpp.Replace("roopm_pop_clip()", "R.pop()");

        cpp = cpp.Replace("roopm_is_rect_clicked(", "R.clicked(");
        cpp = cpp.Replace("roopm_is_rect_pressed(", "R.pressed(");
        cpp = cpp.Replace("roopm_is_rect_hovered(", "R.hovered(");
        cpp = cpp.Replace("roopm_is_pointer_in_rect(", "R.inRect(");

        cpp = Regex.Replace(cpp, @"RoopmInput\s*\*input\s*=\s*roopm_get_input\(\);", "");
        cpp = cpp.Replace("input->", "g_input.");
        cpp = cpp.Replace("g_input.pointerX", "g_input.mouseX");
        cpp = cpp.Replace("g_input.pointerY", "g_input.mouseY");
        cpp = cpp.Replace("g_input.pointerDown", "g_input.mouseDown");

        cpp = cpp.Replace("ROOPM_RGBA", "R.rgba");
        cpp = cpp.Replace("ROOPM_RGB", "R.rgb");
        cpp = cpp.Replace("ROOPM_WHITE", "'#ffffff'");

        // String Formatting (All buffers mapped explicitly!)
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""Taps: %d"",\s*(\w+)\);", "$1 = 'Taps: ' + $2;");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""Tap Me""\);", "$1 = 'Tap Me';");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""%d"",\s*(\w+)\);", "$1 = '' + $2;");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""%.0f%%"",\s*(\w+)\);", "$1 = '' + Math.round($2) + '%';");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""Slider Progress: %.0f%%"",\s*([\w\*\.\s]+)\);", "$1 = 'Slider Progress: ' + Math.round($2) + '%';");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""Logical Viewport: %.0f x %.0f dp"",\s*(\w+),\s*(\w+)\);", "$1 = 'Logical Viewport: ' + Math.round($2) + ' x ' + Math.round($3) + ' dp';");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""Physical Resolution: %d x %d px""[\s\S]*?\);", "$1 = 'Physical Resolution: ' + canvas.width + ' x ' + canvas.height + ' px';");
        cpp = Regex.Replace(cpp, @"snprintf\((\w+),\s*sizeof\(\w+\),\s*""DPI Scale: %\.2fx.*?\);", "$1 = 'DPI Scale: ' + (window.devicePixelRatio||1).toFixed(2) + 'x | Mode: ' + (isLandscape ? 'Landscape' : 'Portrait');");
        cpp = Regex.Replace(cpp, @"snprintf\(dpiStatusBuf,\s*sizeof\(dpiStatusBuf\),\s*""%\.2fx"",\s*dpi\);", "dpiStatusBuf = (window.devicePixelRatio||1).toFixed(2) + 'x';");
        cpp = Regex.Replace(cpp, @"snprintf\(.*?\);", "// Formatted", RegexOptions.Singleline);

        return cpp;
    }

    static void GenerateStyleCSS(string dir)
    {
        string css = @"
* { margin: 0; padding: 0; box-sizing: border-box; }
body { 
    background-color: #0b1120; 
    overflow: hidden; 
    touch-action: none; 
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
}
canvas { display: block; outline: none; }
";
        File.WriteAllText(Path.Combine(dir, "style.css"), css);
    }

    static void GenerateRoopmJS(string dir, string logic, string staticVars)
    {
        string js = @"
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

let g_lastTime = performance.now();

// C++ Global State
" + staticVars + @"

let g_input = { 
    deltaTime: 0, 
    scrollDeltaY: 0,
    mouseX: 0,
    mouseY: 0,
    mouseDown: false,
    pointerJustPressed: false,
    pointerJustReleased: false
};

const R = {
    clear: function(c) { ctx.fillStyle=c; ctx.fillRect(0,0,canvas.width,canvas.height); },
    rect: function(x,y,w,h,c) { ctx.fillStyle=c; ctx.fillRect(x,y,w,h); },
    outline: function(x,y,w,h,c,t) { ctx.strokeStyle=c; ctx.lineWidth=t||1; ctx.strokeRect(x,y,w,h); },
    rectRounded: function(x,y,w,h,r,c) {
        if(w <= 0 || h <= 0) return;
        ctx.fillStyle = c;
        ctx.beginPath();
        if (ctx.roundRect) ctx.roundRect(x, y, w, h, r); else ctx.rect(x, y, w, h);
        ctx.fill();
    },
    rectRoundedGradient: function(x,y,w,h,r,c1,c2) {
        if(w <= 0 || h <= 0) return;
        let grad = ctx.createLinearGradient(x, y, x, y + h);
        grad.addColorStop(0, c1);
        grad.addColorStop(1, c2);
        ctx.fillStyle = grad;
        ctx.beginPath();
        if (ctx.roundRect) ctx.roundRect(x, y, w, h, r); else ctx.rect(x, y, w, h);
        ctx.fill();
    },
    outlineRounded: function(x,y,w,h,r,c,t) {
        if(w <= 0 || h <= 0) return;
        ctx.strokeStyle = c; ctx.lineWidth = t||1;
        ctx.beginPath();
        if (ctx.roundRect) ctx.roundRect(x, y, w, h, r); else ctx.rect(x, y, w, h);
        ctx.stroke();
    },
    circle: function(x,y,r,c) { ctx.fillStyle=c; ctx.beginPath(); ctx.arc(x,y,r,0,Math.PI*2); ctx.fill(); },
    circleOutline: function(x,y,r,c,t) { ctx.strokeStyle=c; ctx.lineWidth=t||1; ctx.beginPath(); ctx.arc(x,y,r,0,Math.PI*2); ctx.stroke(); },
    line: function(x1,y1,x2,y2,c,t) { ctx.strokeStyle=c; ctx.lineWidth=t||1; ctx.beginPath(); ctx.moveTo(x1,y1); ctx.lineTo(x2,y2); ctx.stroke(); },
    text: function(x,y,t,s,c) { 
        ctx.fillStyle=c; ctx.font= s + 'px Arial, sans-serif'; 
        ctx.fillText(t,x,y+s*0.82); 
    },
    textCentered: function(x,y,w,h,t,s,c) {
        ctx.fillStyle=c; ctx.font= s + 'px Arial, sans-serif';
        let tw = ctx.measureText(t).width;
        let tx = x + (w - tw) * 0.5;
        let ty = y + (h - s) * 0.5;
        ctx.fillText(t, tx, ty + s * 0.82);
    },
    textWrapped: function(x, y, maxW, text, size, lineHeight, color) {
        if (!text || maxW <= 0) return 0;
        ctx.fillStyle = color;
        ctx.font = size + 'px Arial, sans-serif';
        let words = text.split(' ');
        let line = '';
        let curY = y;
        for (let n = 0; n < words.length; n++) {
            let testLine = line + (line ? ' ' : '') + words[n];
            let metrics = ctx.measureText(testLine);
            if (metrics.width > maxW && n > 0) {
                ctx.fillText(line, x, curY + size * 0.82);
                line = words[n];
                curY += lineHeight;
            } else {
                line = testLine;
            }
        }
        ctx.fillText(line, x, curY + size * 0.82);
        return (curY + lineHeight) - y;
    },
    measureTextWrappedHeight: function(maxW, text, size, lineHeight) {
        if (!text || maxW <= 0) return 0;
        ctx.font = size + 'px Arial, sans-serif';
        let words = text.split(' ');
        let line = '';
        let totalH = 0;
        for (let n = 0; n < words.length; n++) {
            let testLine = line + (line ? ' ' : '') + words[n];
            let metrics = ctx.measureText(testLine);
            if (metrics.width > maxW && n > 0) {
                totalH += lineHeight;
                line = words[n];
            } else {
                line = testLine;
            }
        }
        if (line) totalH += lineHeight;
        return totalH;
    },
    measureText: function(t, s) {
        ctx.font = s + 'px Arial, sans-serif';
        return ctx.measureText(t).width;
    },
    clip: function(x,y,w,h) { ctx.save(); ctx.beginPath(); ctx.rect(x,y,w,h); ctx.clip(); },
    pop: function() { ctx.restore(); },
    inRect: function(x,y,w,h) { return g_input.mouseX>=x && g_input.mouseX<=x+w && g_input.mouseY>=y && g_input.mouseY<=y+h; },
    clicked: function(x,y,w,h) { return g_input.pointerJustReleased && R.inRect(x,y,w,h); },
    pressed: function(x,y,w,h) { return g_input.mouseDown && R.inRect(x,y,w,h); },
    hovered: function(x,y,w,h) { return R.inRect(x,y,w,h); },
    rgb: function(r,g,b) { return 'rgb(' + r + ',' + g + ',' + b + ')'; },
    rgba: function(r,g,b,a) { return 'rgba(' + r + ',' + g + ',' + b + ',' + (a/255.0) + ')'; }
};

window.onresize = function() {
    var dpr = window.devicePixelRatio || 1;
    var w = window.innerWidth;
    var h = window.innerHeight;
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    canvas.style.width = w + 'px';
    canvas.style.height = h + 'px';
    ctx.scale(dpr,dpr);
};
window.onresize();

function updateMouse(x, y) {
    g_input.mouseX = x; 
    g_input.mouseY = y;
}

canvas.onmousemove = function(e) { 
    var r = canvas.getBoundingClientRect(); 
    updateMouse(e.clientX - r.left, e.clientY - r.top);
};
canvas.onmousedown = function(e) { 
    g_input.mouseDown = true; 
    g_input.pointerJustPressed = true;
    var r = canvas.getBoundingClientRect(); 
    updateMouse(e.clientX - r.left, e.clientY - r.top);
};
canvas.onmouseup = function(e) { 
    g_input.mouseDown = false; 
    g_input.pointerJustReleased = true;
};
canvas.onwheel = function(e) { 
    e.preventDefault(); 
    g_input.scrollDeltaY = e.deltaY * 0.025; 
};

canvas.ontouchstart = function(e) {
    if(e.cancelable) e.preventDefault();
    var t=e.touches[0]; var r=canvas.getBoundingClientRect();
    updateMouse(t.clientX-r.left, t.clientY-r.top);
    g_input.mouseDown = true;
    g_input.pointerJustPressed = true;
};
canvas.ontouchmove = function(e) {
    if(e.cancelable) e.preventDefault();
    var t=e.touches[0]; var r=canvas.getBoundingClientRect();
    updateMouse(t.clientX-r.left, t.clientY-r.top);
};
canvas.ontouchend = function(e) { 
    if(e.cancelable) e.preventDefault(); 
    g_input.mouseDown = false; 
    g_input.pointerJustReleased = true;
};

function update() {
    var now = performance.now();
    g_input.deltaTime = (now - g_lastTime) / 1000.0;
    if (g_input.deltaTime > 0.05) g_input.deltaTime = 0.05;
    if (g_input.deltaTime <= 0.0) g_input.deltaTime = 0.016;
    g_lastTime = now;

    // --- C++ Logic Start ---
    " + logic + @"
    // --- C++ Logic End ---

    g_input.scrollDeltaY = 0;
    g_input.pointerJustPressed = false;
    g_input.pointerJustReleased = false;
    requestAnimationFrame(update);
}
update();
";
        File.WriteAllText(Path.Combine(dir, "roopm.js"), js);
    }

    static void GenerateIndexHTML(string dir)
    {
        string html = @"<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>
    <title>RoopM - Pure C++ Web App</title>
    <link rel='stylesheet' href='style.css'>
</head>
<body>
    <canvas id='canvas'></canvas>
    <script src='roopm.js'></script>
</body>
</html>";
        File.WriteAllText(Path.Combine(dir, "index.html"), html);
    }
}