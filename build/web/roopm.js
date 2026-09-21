
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

let g_lastTime = performance.now();

// C++ Global State
let g_clickCount = 0;
let g_stepperValue = 42;
let g_stepperHoldTimer = 0.0;
let g_isDarkMode = false;
let g_scrollY = 0.0;
let g_targetScrollY = 0.0;
let g_scrollVelocity = 0.0;
let g_lastTouchY = 0.0;
let g_isTouchDragging = false;
let g_animTime = 0.0;
let g_sliderValue = 0.72;
let g_isDraggingSlider = false;
let g_sliderKnobScale = 1.0;
let g_toggleState = true;
let g_toggleAnim = 1.0;
let g_selectedTab = 0;
let g_tabPillAnimX = 0.0;
let g_selectedListItem = 0;
let g_accentThemeIndex = 0;
let g_chartMode = 0;
let g_isScrubbingChart = false;
let g_chartHoverIndex = -1;
let g_scrubberAnimX = 0.0;
let g_scrubberAnimY = 0.0;
let g_scrubberAnimVal = 0.0;


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
    
  
  let w = window.innerWidth;
  let h = window.innerHeight;
  let dpi = (window.devicePixelRatio||1);

  let isMobile = (dpi > 2.0);
  let isLandscape = isMobile && (w > h * 1.3);

  // 🎯 SAFE AREA
  let topSafeArea = (!isLandscape && isMobile) ? 54.0 : 0.0;
  let bottomSafeArea = (!isLandscape && isMobile) ? 16.0 : 0.0;

  let headerHeight = isLandscape ? 42.0 : (74.0 + topSafeArea);
  let footerHeight = isLandscape ? 24.0 : (36.0 + bottomSafeArea);

  g_animTime += g_input.deltaTime;

  // Active Accent Palette
  let primaryAccents = [
      R.rgb(37, 99, 235),   
      R.rgb(16, 185, 129),  
      R.rgb(139, 92, 246),  
      R.rgb(244, 63, 94)    
  ];
  let primaryDark = [
      R.rgb(29, 78, 216),
      R.rgb(5, 150, 105),
      R.rgb(124, 58, 237),
      R.rgb(225, 29, 72)
  ];
  let primaryLight = [
      R.rgb(239, 246, 255),
      R.rgb(236, 253, 245),
      R.rgb(245, 243, 255),
      R.rgb(255, 241, 242)
  ];

  let activeAccent = primaryAccents[g_accentThemeIndex % 4];
  let activeAccentDark = primaryDark[g_accentThemeIndex % 4];
  let activeAccentLight = primaryLight[g_accentThemeIndex % 4];

  // Dynamic Theme Colors
  let colBg       = g_isDarkMode ? R.rgb(11, 17, 32)   : R.rgb(244, 246, 249);
  let colCard     = g_isDarkMode ? R.rgb(30, 41, 59)   : '#ffffff';
  let colBorder   = g_isDarkMode ? R.rgb(51, 65, 85)   : R.rgb(226, 232, 240);
  let colTextHead = g_isDarkMode ? R.rgb(248, 250, 252): R.rgb(15, 23, 42);
  let colTextBody = g_isDarkMode ? R.rgb(148, 163, 184): R.rgb(100, 116, 139);
  let colSubBg    = g_isDarkMode ? R.rgb(15, 23, 42)   : R.rgb(241, 245, 249);

  let sidePadding = isLandscape ? 12.0 : (w < 300.0 ? 8.0 : 14.0);
  let cardWidth = w - (sidePadding * 2.0);

  let fTitle = isLandscape ? 10.0 : (cardWidth < 240.0 ? 14.0 : 16.5);
  let fBody  = isLandscape ? 7.5  : (cardWidth < 240.0 ? 11.0 : 12.5);
  let fSmall = isLandscape ? 6.5  : 10.5;
  let cardInnerW = cardWidth - 24.0;

  // ===========================================================================
  // 📐 INTRINSIC DYNAMIC CARD HEIGHT CALCULATIONS
  // ===========================================================================
  
  let heroTitleH = R.measureTextWrappedHeight(cardInnerW, "RoopM Universal Engine", fTitle + 2.0, (fTitle + 2.0) * 1.25);
  let heroDescH = R.measureTextWrappedHeight(cardInnerW, "100% Pure C++ software rendering framework. Native performance with sub-pixel TrueType typography.", fBody, fBody * 1.35);
  let heroAccentLabelW = R.measureText("Live Accent:", fSmall);
  let chipsNeedStack = (heroAccentLabelW + 8.0 + 96.0 > cardInnerW);
  let heroChipsH = chipsNeedStack ? (fSmall + 6.0 + 20.0) : 22.0;
  let chHeroBanner = 14.0 + heroTitleH + 6.0 + heroDescH + 10.0 + heroChipsH + 14.0;

  let c2TitleH = R.measureTextWrappedHeight(cardInnerW, "Interactive Actions & Stepper", fTitle, fTitle * 1.25);
  let btnMinW = isLandscape ? 75.0 : 95.0;
  let stepUnitW = isLandscape ? 22.0 : 30.0;
  let valUnitW = isLandscape ? 30.0 : 38.0;
  let stepperTotalW = (stepUnitW * 2.0) + valUnitW + 8.0;
  let stepperNeedsStack = (btnMinW + 10.0 + stepperTotalW > cardInnerW);
  let chButtonRow = 12.0 + c2TitleH + 10.0 + (stepperNeedsStack ? (36.0 + 8.0 + 36.0) : 36.0) + 14.0;

  let modeBtnW = isLandscape ? 38.0 : 44.0;
  let modeBtnH = isLandscape ? 14.0 : 20.0;
  let modeBtnsTotalW = (modeBtnW * 2.0) + 4.0;
  let fullChartTitleW = R.measureText("Live Metrics Graph", fTitle);
  let chartHeaderNeedsStack = (fullChartTitleW + 12.0 + modeBtnsTotalW > cardInnerW);
  let c3TitleH = R.measureTextWrappedHeight(chartHeaderNeedsStack ? cardInnerW : (cardInnerW - modeBtnsTotalW - 12.0), "Live Metrics Graph", fTitle, fTitle * 1.25);
  let chartH = isLandscape ? 80.0 : 115.0;
  let chLiveChart = 12.0 + (chartHeaderNeedsStack ? (c3TitleH + 6.0 + modeBtnH + 10.0) : (Math.max(c3TitleH, modeBtnH) + 10.0)) + chartH + 14.0;

  let c4TitleH = R.measureTextWrappedHeight(cardInnerW, "Advanced UI Controls", fTitle, fTitle * 1.25);
  let toggleSleekH = isLandscape ? 15.0 : 20.0;
  let chControls = 12.0 + c4TitleH + 10.0 + 20.0 + 14.0 + toggleSleekH + 16.0;

  let c5TitleH = R.measureTextWrappedHeight(cardInnerW, "System View Selector", fTitle, fTitle * 1.25);
  let chTabsList = 12.0 + c5TitleH + 8.0 + 30.0 + 8.0 + (3 * 38.0) + 14.0;

  let c6TitleH = R.measureTextWrappedHeight(cardInnerW, "Fluid Physics Wave (Touch Reactive)", fTitle, fTitle * 1.25);
  let chShapes = 12.0 + c6TitleH + (isLandscape ? 16.0 : 24.0) + 30.0 + 14.0;

  // Card 7 Diagnostics
  let c7TitleH = R.measureTextWrappedHeight(cardInnerW, "System Diagnostics", fTitle, fTitle * 1.25);
  let diagText1 = '';
  let diagText2 = '';
  let diagText3 = '';
  diagText1 = 'Logical Viewport: ' + Math.round(w) + ' x ' + Math.round(h) + ' dp';
  diagText2 = 'Physical Resolution: ' + canvas.width + ' x ' + canvas.height + ' px';
  diagText3 = 'DPI Scale: ' + (window.devicePixelRatio||1).toFixed(2) + 'x | Mode: ' + (isLandscape ? 'Landscape' : 'Portrait');
  let dH1 = R.measureTextWrappedHeight(cardInnerW, diagText1, fSmall, fSmall * 1.35);
  let dH2 = R.measureTextWrappedHeight(cardInnerW, diagText2, fSmall, fSmall * 1.35);
  let dH3 = R.measureTextWrappedHeight(cardInnerW, diagText3, fSmall, fSmall * 1.35);
  let chMetrics = 12.0 + c7TitleH + 6.0 + dH1 + 4.0 + dH2 + 4.0 + dH3 + 14.0;

  // Card 8 Highlights
  let c8TitleH = R.measureTextWrappedHeight(cardInnerW, "RoopM Architecture", fTitle, fTitle * 1.25);
  let aH1 = R.measureTextWrappedHeight(cardInnerW, "+ 100% Software Renderer with Sub-pixel AA", fSmall, fSmall * 1.35);
  let aH2 = R.measureTextWrappedHeight(cardInnerW, "+ TrueType direct font rasterization", fSmall, fSmall * 1.35);
  let aH3 = R.measureTextWrappedHeight(cardInnerW, "+ Unified Framebuffer & Cross-Platform Input", fSmall, fSmall * 1.35);
  let chHighlights = 12.0 + c8TitleH + 6.0 + aH1 + 4.0 + aH2 + 4.0 + aH3 + 14.0;

  let cardSpacing = isLandscape ? 8.0 : 14.0;
  let totalContentHeight = chHeroBanner + chButtonRow + chLiveChart + chControls + chTabsList + chShapes + chMetrics + chHighlights + (9 * cardSpacing) + 30.0;
  let visibleHeight = h - headerHeight - footerHeight;
  let maxScroll = totalContentHeight - visibleHeight;
  if (maxScroll < 0.0) maxScroll = 0.0;

  // ===========================================================================
  // 🎯 SCROLLING & TOUCH INTERACTION
  // ===========================================================================

  let contentY = headerHeight + (isLandscape ? 8.0 : 14.0) - g_scrollY;
  let cardControlsY = contentY + chHeroBanner + chButtonRow + chLiveChart + (3 * cardSpacing);
  let sY = cardControlsY + 12.0 + c4TitleH + 10.0;
  let sPad = isMobile ? 24.0 : 12.0;

  let chartCardY = contentY + chHeroBanner + chButtonRow + (2 * cardSpacing);
  let chartX = sidePadding + 12.0;
  let chartY = chartCardY + 12.0 + (chartHeaderNeedsStack ? (c3TitleH + 6.0 + modeBtnH + 10.0) : (Math.max(c3TitleH, modeBtnH) + 10.0));
  let chartW = cardInnerW;

  if (g_input.pointerJustPressed) {
    if (R.inRect(sidePadding + 8.0, sY - sPad, cardWidth - 16.0, 24.0 + sPad * 2.0)) {
      g_isDraggingSlider = true;
      g_isTouchDragging = false;
      g_isScrubbingChart = false;
    } else if (R.inRect(chartX, chartY, chartW, chartH)) {
      g_isScrubbingChart = true;
      g_isDraggingSlider = false;
      g_isTouchDragging = false;
    } else {
      g_isDraggingSlider = false;
      g_isScrubbingChart = false;
      g_isTouchDragging = true;
      g_lastTouchY = g_input.mouseY;
      g_scrollVelocity = 0.0;
    }
  }

  if (g_input.mouseDown) {
    if (g_isTouchDragging) {
      let rawDeltaY = g_lastTouchY - g_input.mouseY;
      let resistance = 1.0;
      if (g_scrollY < 0.0) {
        resistance = Math.exp(g_scrollY * 0.012);
      } else if (g_scrollY > maxScroll) {
        resistance = Math.exp((maxScroll - g_scrollY) * 0.012);
      }
      
      let appliedDeltaY = rawDeltaY * resistance;
      g_scrollY += appliedDeltaY;
      g_targetScrollY = g_scrollY;

      if (g_input.deltaTime > 0.0001) {
        let instVel = rawDeltaY / g_input.deltaTime;
        g_scrollVelocity = (g_scrollVelocity * 0.4) + (instVel * 0.6);
      }
      g_lastTouchY = g_input.mouseY;
    }
  }

  if (g_input.pointerJustReleased) {
    g_isDraggingSlider = false;
    g_isScrubbingChart = false;
    g_isTouchDragging = false;
    g_chartHoverIndex = -1;

    if (Math.abs(g_scrollVelocity) < 60.0) g_scrollVelocity = 0.0;
    if (g_scrollVelocity > 3500.0) g_scrollVelocity = 3500.0;
    if (g_scrollVelocity < -3500.0) g_scrollVelocity = -3500.0;
  }

  if (g_input.scrollDeltaY != 0.0 && !g_isTouchDragging) {
    g_targetScrollY += g_input.scrollDeltaY * 54.0;
    g_scrollVelocity = 0.0;
    if (g_targetScrollY < 0.0) g_targetScrollY = 0.0;
    if (g_targetScrollY > maxScroll) g_targetScrollY = maxScroll;
  }

  if (!g_isTouchDragging) {
    if (g_scrollY < 0.0) {
      let springFactor = 1.0 - Math.exp(-14.0 * g_input.deltaTime);
      g_scrollY += (0.0 - g_scrollY) * springFactor;
      g_scrollVelocity = 0.0;
      g_targetScrollY = 0.0;
    } else if (g_scrollY > maxScroll) {
      let springFactor = 1.0 - Math.exp(-14.0 * g_input.deltaTime);
      g_scrollY += (maxScroll - g_scrollY) * springFactor;
      g_scrollVelocity = 0.0;
      g_targetScrollY = maxScroll;
    } else if (Math.abs(g_scrollVelocity) > 1.0) {
      g_scrollY += g_scrollVelocity * g_input.deltaTime;
      g_targetScrollY = g_scrollY;
      let friction = Math.exp(-3.6 * g_input.deltaTime);
      g_scrollVelocity *= friction;
    } else {
      g_scrollVelocity = 0.0;
      let smoothFactor = 1.0 - Math.exp(-12.0 * g_input.deltaTime);
      g_scrollY += (g_targetScrollY - g_scrollY) * smoothFactor;
    }
  }

  // Micro-Animation Springs
  let targetToggle = g_toggleState ? 1.0 : 0.0;
  g_toggleAnim += (targetToggle - g_toggleAnim) * (1.0 - Math.exp(-20.0 * g_input.deltaTime));

  let targetKnobScale = g_isDraggingSlider ? 1.35 : 1.0;
  g_sliderKnobScale += (targetKnobScale - g_sliderKnobScale) * (1.0 - Math.exp(-22.0 * g_input.deltaTime));

  // ===========================================================================
  // RENDERING PIPELINE
  // ===========================================================================

  R.clear(colBg);
  R.clip(0, headerHeight, w, visibleHeight);

  let cardRadius = isLandscape ? 6.0 : 12.0;
  contentY = headerHeight + (isLandscape ? 8.0 : 14.0) - g_scrollY;

  // --- CARD 1: HERO GRADIENT BANNER ---
  let cardY = contentY;
  R.rectRoundedGradient(sidePadding, cardY, cardWidth, chHeroBanner, cardRadius, activeAccent, activeAccentDark);
  
  let heroY = cardY + 14.0;
  heroY += R.textWrapped(sidePadding + 12.0, heroY, cardInnerW, "RoopM Universal Engine", fTitle + 2.0, (fTitle + 2.0) * 1.25, '#ffffff') + 6.0;
  heroY += R.textWrapped(sidePadding + 12.0, heroY, cardInnerW, 
                            "100% Pure C++ software rendering framework. Native performance with sub-pixel TrueType typography.", 
                            fBody, fBody * 1.35, R.rgba(255, 255, 255, 230)) + 10.0;

  let chipY = heroY;
  if (chipsNeedStack) {
    R.text(sidePadding + 12.0, chipY, "Live Accent:", fSmall, '#ffffff');
    chipY += fSmall + 6.0;
    for (let c = 0; c < 4; c++) {
      let chipX = sidePadding + 12.0 + (c * 24.0);
      if (R.clicked(chipX - 2.0, chipY - 2.0, 20.0, 20.0)) g_accentThemeIndex = c;
      R.circle(chipX + 7.0, chipY + 7.0, 7.0, primaryAccents[c]);
      if (g_accentThemeIndex == c) R.circleOutline(chipX + 7.0, chipY + 7.0, 8.5, '#ffffff', 2.0);
    }
  } else {
    R.text(sidePadding + 12.0, chipY + 2.0, "Live Accent:", fSmall, '#ffffff');
    let chipsStartX = sidePadding + 12.0 + heroAccentLabelW + 8.0;
    for (let c = 0; c < 4; c++) {
      let chipX = chipsStartX + (c * 24.0);
      if (R.clicked(chipX - 2.0, chipY - 2.0, 20.0, 20.0)) g_accentThemeIndex = c;
      R.circle(chipX + 7.0, chipY + 7.0, 7.0, primaryAccents[c]);
      if (g_accentThemeIndex == c) R.circleOutline(chipX + 7.0, chipY + 7.0, 8.5, '#ffffff', 2.0);
    }
  }

  // --- CARD 2: ACTIONS & STEPPER ---
  cardY += chHeroBanner + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chButtonRow, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chButtonRow, colBorder, 1.0);
  
  R.textWrapped(sidePadding + 12.0, cardY + 12.0, cardInnerW, "Interactive Actions & Stepper", fTitle, fTitle * 1.25, colTextHead);

  let btnH = isLandscape ? 20.0 : 36.0;
  let stepH = btnH;
  let btnX, btnY, btnW;
  let stepX, stepY, stepW, valBoxX, valBoxW, plusX;

  if (stepperNeedsStack) {
    btnX = sidePadding + 12.0;
    btnY = cardY + 12.0 + c2TitleH + 8.0;
    btnW = cardInnerW;

    stepY = btnY + btnH + 8.0;
    stepW = 32.0;
    valBoxW = 40.0;
    let stepRowTotal = (stepW * 2.0) + valBoxW + 8.0;
    stepX = sidePadding + (cardWidth - stepRowTotal) * 0.5;
    valBoxX = stepX + stepW + 4.0;
    plusX = valBoxX + valBoxW + 4.0;
  } else {
    btnY = cardY + 12.0 + c2TitleH + 10.0;
    stepY = btnY;
    btnX = sidePadding + 12.0;
    btnW = isLandscape ? 80.0 : 110.0;

    let rightMargin = sidePadding + cardWidth - 12.0;
    stepW = stepUnitW;
    valBoxW = valUnitW;

    plusX = rightMargin - stepW;
    valBoxX = plusX - 4.0 - valBoxW;
    stepX = valBoxX - 4.0 - stepW;
  }

  let btnHovered = R.hovered(btnX, btnY, btnW, btnH);
  let btnPressed = R.pressed(btnX, btnY, btnW, btnH);
  if (R.clicked(btnX, btnY, btnW, btnH)) g_clickCount++;

  let btnBg = btnPressed ? activeAccentDark : (btnHovered ? activeAccent : activeAccent);
  R.rectRounded(btnX, btnY, btnW, btnH, isLandscape ? 4.0 : 8.0, btnBg);
  
  let tapBtnBuf = '';
  if (g_clickCount > 0) tapBtnBuf = 'Taps: ' + g_clickCount;
  else tapBtnBuf = 'Tap Me';
  R.textCentered(btnX, btnY, btnW, btnH, tapBtnBuf, isLandscape ? 8.0 : 14.0, '#ffffff');

  let minusPressed = R.pressed(stepX, stepY, stepW, stepH);
  if (R.clicked(stepX, stepY, stepW, stepH)) {
    if (g_stepperValue > 0) g_stepperValue--;
  }
  if (minusPressed) {
    g_stepperHoldTimer += g_input.deltaTime;
    if (g_stepperHoldTimer > 0.5) {
      if (g_stepperValue > 0) g_stepperValue--;
      g_stepperHoldTimer = 0.4;
    }
  }

  R.rectRounded(stepX, stepY, stepW, stepH, isLandscape ? 3.0 : 6.0, minusPressed ? activeAccentLight : colSubBg);
  R.textCentered(stepX, stepY, stepW, stepH, "-", isLandscape ? 10.0 : 16.0, minusPressed ? activeAccentDark : colTextHead);

  let stepBuf = ''; stepBuf = '' + g_stepperValue;
  R.textCentered(valBoxX, stepY, valBoxW, stepH, stepBuf, isLandscape ? 8.0 : 14.0, colTextHead);

  let plusPressed = R.pressed(plusX, stepY, stepW, stepH);
  if (R.clicked(plusX, stepY, stepW, stepH)) {
    g_stepperValue++;
  }
  if (plusPressed) {
    g_stepperHoldTimer += g_input.deltaTime;
    if (g_stepperHoldTimer > 0.5) {
      g_stepperValue++;
      g_stepperHoldTimer = 0.4;
    }
  }
  if (!minusPressed && !plusPressed) g_stepperHoldTimer = 0.0;

  R.rectRounded(plusX, stepY, stepW, stepH, isLandscape ? 3.0 : 6.0, plusPressed ? activeAccentLight : colSubBg);
  R.textCentered(plusX, stepY, stepW, stepH, "+", isLandscape ? 10.0 : 16.0, plusPressed ? activeAccentDark : colTextHead);

  // --- CARD 3: LIVE CHART ---
  cardY += chButtonRow + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chLiveChart, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chLiveChart, colBorder, 1.0);

  let modeBtnX1, modeBtnX2, modeBtnY;

  if (chartHeaderNeedsStack) {
    R.textWrapped(sidePadding + 12.0, cardY + 10.0, cardInnerW, "Live Metrics Graph", fTitle, fTitle * 1.25, colTextHead);
    modeBtnY = cardY + 10.0 + c3TitleH + 6.0;
    modeBtnX1 = sidePadding + 12.0;
    modeBtnX2 = modeBtnX1 + modeBtnW + 4.0;
  } else {
    R.textWrapped(sidePadding + 12.0, cardY + 12.0, cardInnerW - modeBtnsTotalW - 10.0, "Live Metrics Graph", fTitle, fTitle * 1.25, colTextHead);
    modeBtnY = cardY + 10.0;
    modeBtnX2 = sidePadding + cardWidth - 12.0 - modeBtnW;
    modeBtnX1 = modeBtnX2 - modeBtnW - 4.0;
  }

  if (R.clicked(modeBtnX1, modeBtnY, modeBtnW, modeBtnH)) g_chartMode = 0;
  if (R.clicked(modeBtnX2, modeBtnY, modeBtnW, modeBtnH)) g_chartMode = 1;

  R.rectRounded(modeBtnX1, modeBtnY, modeBtnW, modeBtnH, 4.0, g_chartMode == 0 ? activeAccent : colSubBg);
  R.textCentered(modeBtnX1, modeBtnY, modeBtnW, modeBtnH, "Wave", isLandscape ? 5.5 : 9.5, g_chartMode == 0 ? '#ffffff' : colTextBody);

  R.rectRounded(modeBtnX2, modeBtnY, modeBtnW, modeBtnH, 4.0, g_chartMode == 1 ? activeAccent : colSubBg);
  R.textCentered(modeBtnX2, modeBtnY, modeBtnW, modeBtnH, "Bars", isLandscape ? 5.5 : 9.5, g_chartMode == 1 ? '#ffffff' : colTextBody);

  for (let g = 0; g <= 3; g++) {
    let gy = chartY + (g * (chartH / 3.0));
    R.line(chartX, gy, chartX + chartW, gy, colSubBg, 1.0);
  }

  let numPoints = 12;
  let chartPadX = 14.0;
  let chartInnerX = chartX + chartPadX;
  let chartInnerWidth = chartW - (chartPadX * 2.0);
  let slotW = chartInnerWidth / numPoints;
  let maxBarW = isLandscape ? 16.0 : 24.0;
  let barWidth = Math.min(slotW * 0.65, maxBarW);

  let prevX = 0, prevY = 0;
  let targetScrubX = 0, targetScrubY = 0, targetVal = 0;

  if ((g_isScrubbingChart || R.inRect(chartX, chartY - 10.0, chartW, chartH + 20.0)) && g_input.mouseDown) {
    let relX = g_input.mouseX - chartInnerX;
    let nearestIndex = (relX / slotW);
    if (nearestIndex < 0) nearestIndex = 0;
    if (nearestIndex >= numPoints) nearestIndex = numPoints - 1;
    g_chartHoverIndex = nearestIndex;
  }

  for (let p = 0; p < numPoints; p++) {
    let slotCenterX = chartInnerX + (p + 0.5) * slotW;
    
    let wave = (Math.sin(g_animTime * 2.2 + p * 0.45) * 0.3) + (Math.cos(g_animTime * 1.1 + p * 0.7) * 0.2) + 0.5;
    if (wave < 0.08) wave = 0.08;
    if (wave > 0.92) wave = 0.92;
    let py = chartY + chartH - (wave * (chartH - 26.0)) - 12.0;

    if (g_chartMode == 0) {
      if (p > 0) R.line(prevX, prevY, slotCenterX, py, activeAccent, 2.5);
      R.circle(slotCenterX, py, 3.5, activeAccent);
      R.circleOutline(slotCenterX, py, 3.5, '#ffffff', 1.5);
    } else {
      let barH = wave * (chartH - 24.0) + 6.0;
      let barY = chartY + chartH - barH - 2.0;
      let barX = slotCenterX - (barWidth * 0.5);
      R.rectRounded(barX, barY, barWidth, barH, 3.0, (g_chartHoverIndex == p) ? activeAccentDark : activeAccent);
    }

    if (g_chartHoverIndex == p) {
      targetScrubX = slotCenterX;
      targetScrubY = py;
      targetVal = wave * 100.0;
    }

    prevX = slotCenterX;
    prevY = py;
  }

  if (g_chartHoverIndex >= 0) {
    if (g_scrubberAnimX == 0.0) {
      g_scrubberAnimX = targetScrubX;
      g_scrubberAnimY = targetScrubY;
      g_scrubberAnimVal = targetVal;
    } else {
      let lerpSpeed = 1.0 - Math.exp(-25.0 * g_input.deltaTime);
      g_scrubberAnimX += (targetScrubX - g_scrubberAnimX) * lerpSpeed;
      g_scrubberAnimY += (targetScrubY - g_scrubberAnimY) * lerpSpeed;
      g_scrubberAnimVal += (targetVal - g_scrubberAnimVal) * lerpSpeed;
    }

    R.line(g_scrubberAnimX, chartY, g_scrubberAnimX, chartY + chartH, R.rgb(148, 163, 184), 1.0);
    R.circle(g_scrubberAnimX, g_scrubberAnimY, 6.0, activeAccentDark);
    R.circleOutline(g_scrubberAnimX, g_scrubberAnimY, 6.0, '#ffffff', 2.0);

    let tipW = 46.0;
    let tipH = 18.0;
    let tipX = g_scrubberAnimX - tipW * 0.5;
    let tipY = g_scrubberAnimY - tipH - 6.0;
    if (tipX < chartX) tipX = chartX;
    if (tipX + tipW > chartX + chartW) tipX = chartX + chartW - tipW;

    R.rectRounded(tipX, tipY, tipW, tipH, 4.0, R.rgb(15, 23, 42));
    let tipBuf = ''; tipBuf = '' + Math.round(g_scrubberAnimVal) + '%';
    R.textCentered(tipX, tipY, tipW, tipH, tipBuf, 9.0, '#ffffff');
  } else {
    g_scrubberAnimX = 0.0;
  }

  // --- CARD 4: ADVANCED UI CONTROLS ---
  cardY += chLiveChart + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chControls, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chControls, colBorder, 1.0);
  
  R.textWrapped(sidePadding + 12.0, cardY + 12.0, cardInnerW, "Advanced UI Controls", fTitle, fTitle * 1.25, colTextHead);

  let sliderX = sidePadding + 12.0;
  let sliderY = cardY + 12.0 + c4TitleH + 10.0;
  let sliderW = cardInnerW;
  let sliderH = isLandscape ? 12.0 : 20.0;

  if (g_isDraggingSlider && g_input.mouseDown) {
    let relativeX = g_input.mouseX - sliderX;
    g_sliderValue = relativeX / sliderW;
    if (g_sliderValue < 0.0) g_sliderValue = 0.0;
    if (g_sliderValue > 1.0) g_sliderValue = 1.0;
  }

  let trackHeight = isLandscape ? 4.0 : 6.0;
  let trackY = sliderY + (sliderH * 0.5) - (trackHeight * 0.5);
  R.rectRounded(sliderX, trackY, sliderW, trackHeight, trackHeight * 0.5, colSubBg);

  for (let t = 0; t <= 4; t++) {
    let tickX = sliderX + (t * (sliderW / 4.0));
    R.circle(tickX, sliderY + (sliderH * 0.5), 2.5, colBorder);
  }

  let fillW = g_sliderValue * sliderW;
  R.rectRounded(sliderX, trackY, fillW, trackHeight, trackHeight * 0.5, activeAccent);

  let knobX = sliderX + fillW;
  let knobY = sliderY + (sliderH * 0.5);
  let knobR = (isLandscape ? 6.5 : 9.5) * g_sliderKnobScale;
  R.circle(knobX, knobY, knobR, activeAccent);
  R.circleOutline(knobX, knobY, knobR, '#ffffff', 2.5);

  let sliderLabel = '';
  sliderLabel = 'Slider Progress: ' + Math.round(g_sliderValue * 100.0) + '%';
  R.text(sliderX, sliderY + sliderH + (isLandscape ? 3.0 : 6.0), sliderLabel, fSmall, colTextBody);

  let toggleX = sidePadding + 12.0;
  let toggleY = sliderY + sliderH + (isLandscape ? 18.0 : 24.0);
  let toggleW = isLandscape ? 28.0 : 36.0;
  let toggleH = toggleSleekH;

  if (R.clicked(toggleX - 5.0, toggleY - 5.0, toggleW + 120.0, toggleH + 10.0)) {
    g_toggleState = !g_toggleState;
  }

  let toggleBg = g_toggleState ? R.rgb(16, 185, 129) : colSubBg;
  R.rectRounded(toggleX, toggleY, toggleW, toggleH, toggleH * 0.5, toggleBg);

  let knobPad = 2.0;
  let toggleKnobSize = toggleH - (knobPad * 2.0);
  let toggleTravel = toggleW - toggleKnobSize - (knobPad * 2.0);
  let toggleKnobX = toggleX + knobPad + (g_toggleAnim * toggleTravel);
  R.rectRounded(toggleKnobX, toggleY + knobPad, toggleKnobSize, toggleKnobSize, toggleKnobSize * 0.5, '#ffffff');

  let toggleTextMaxW = cardInnerW - toggleW - 10.0;
  let toggleLabelY = toggleY + (toggleH - fBody) * 0.5;
  R.textWrapped(toggleX + toggleW + 10.0, toggleLabelY, toggleTextMaxW, 
                    g_toggleState ? "Engine Active (Ready)" : "Engine Suspended", fBody, fBody * 1.25, colTextHead);

  // --- CARD 5: MULTI-VIEW TAB SYSTEM (🌟 Connected Dynamic Buffers) ---
  cardY += chControls + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chTabsList, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chTabsList, colBorder, 1.0);
  
  R.textWrapped(sidePadding + 12.0, cardY + 12.0, cardInnerW, "System View Selector", fTitle, fTitle * 1.25, colTextHead);

  let tabRowX = sidePadding + 12.0;
  let tabRowY = cardY + 12.0 + c5TitleH + 6.0;
  let tabRowW = cardInnerW;
  let tabRowH = isLandscape ? 18.0 : 30.0;
  let singleTabW = tabRowW / 3.0;

  R.rectRounded(tabRowX, tabRowY, tabRowW, tabRowH, tabRowH * 0.5, colSubBg);

  let targetPillX = tabRowX + (g_selectedTab * singleTabW);
  if (g_tabPillAnimX == 0.0) g_tabPillAnimX = targetPillX;
  g_tabPillAnimX += (targetPillX - g_tabPillAnimX) * (1.0 - Math.exp(-20.0 * g_input.deltaTime));
  R.rectRounded(g_tabPillAnimX + 2.0, tabRowY + 2.0, singleTabW - 4.0, tabRowH - 4.0, (tabRowH - 4.0) * 0.5, activeAccent);

  let tabAnalyticsW = R.measureText("Analytics", fSmall);
  let tabsNeedShortNames = (tabAnalyticsW + 6.0 > singleTabW);
  
  let tabNames = [ "Analytics", "Hardware", "Engine" ];
  if (tabsNeedShortNames) {
    tabNames[0] = "Stats";
    tabNames[1] = "HW";
    tabNames[2] = "Core";
  }

  let tabFont = Math.min(fSmall, (singleTabW - 4.0) / 5.2);
  if (tabFont < 8.0) tabFont = 8.0;

  for (let i = 0; i < 3; i++) {
    let tx = tabRowX + (i * singleTabW);
    if (R.clicked(tx, tabRowY, singleTabW, tabRowH)) {
      g_selectedTab = i;
    }
    R.textCentered(tx, tabRowY, singleTabW, tabRowH, tabNames[i], tabFont, (g_selectedTab == i) ? '#ffffff' : colTextBody);
  }

  let rowStartY = tabRowY + tabRowH + (isLandscape ? 6.0 : 8.0);
  let rowH = isLandscape ? 20.0 : 34.0;

  // 🌟 Dynamic DPI Buffer connected properly!
  let dpiStatusBuf = '';
  dpiStatusBuf = (window.devicePixelRatio||1).toFixed(2) + 'x';

  let labels = [ "Frame Budget", "Cache Hits Ratio", "VSync Refresh" ];
  let status = [ "16.6 ms", "99.4 %", "Locked" ];

  if (g_selectedTab == 1) {
    labels[0] = "DPI Scale"; labels[1] = "CPU Threads"; labels[2] = "Framebuffer";
    status[0] = dpiStatusBuf; status[1] = "Multi"; status[2] = "Direct";
  } else if (g_selectedTab == 2) {
    labels[0] = "Sub-pixel AA"; labels[1] = "Direct Blit"; labels[2] = "Memory Footprint";
    status[0] = "Active"; status[1] = "Zero Copy"; status[2] = "< 2 MB";
  }

  for (let r = 0; r < 3; r++) {
    let ry = rowStartY + (r * (rowH + (isLandscape ? 4.0 : 6.0)));
    if (R.clicked(tabRowX, ry, tabRowW, rowH)) {
      g_selectedListItem = r;
    }
    let isSelected = (g_selectedListItem == r);
    let rowBg = isSelected ? (g_isDarkMode ? R.rgb(30, 58, 138) : activeAccentLight) : colSubBg;
    R.rectRounded(tabRowX, ry, tabRowW, rowH, isLandscape ? 3.0 : 6.0, rowBg);

    let badgeW = isLandscape ? 36.0 : (cardInnerW < 160.0 ? 40.0 : 50.0);
    let badgeH = isLandscape ? 12.0 : 18.0;
    let badgeX = tabRowX + tabRowW - badgeW - 6.0;
    let badgeY = ry + (rowH - badgeH) * 0.5;

    let rowTextY = ry + (rowH - fSmall) * 0.5;
    let maxLabelSpan = badgeX - (tabRowX + 8.0) - 6.0;

    R.clip(tabRowX + 8.0, ry, maxLabelSpan, rowH);
    R.text(tabRowX + 8.0, rowTextY, labels[r], fSmall, isSelected ? (g_isDarkMode ? '#ffffff' : activeAccentDark) : colTextHead);
    R.pop();

    R.rectRounded(badgeX, badgeY, badgeW, badgeH, badgeH * 0.5, isSelected ? activeAccent : colBorder);
    R.textCentered(badgeX, badgeY, badgeW, badgeH, status[r], isLandscape ? 5.5 : 9.0, isSelected ? '#ffffff' : colTextBody);
  }

  // --- CARD 6: FLUID WAVE ---
  cardY += chTabsList + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chShapes, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chShapes, colBorder, 1.0);
  
  R.textWrapped(sidePadding + 12.0, cardY + 12.0, cardInnerW, "Fluid Physics Wave (Touch Reactive)", fTitle, fTitle * 1.25, colTextHead);

  let waveColors = [
      R.rgb(239, 68, 68), R.rgb(249, 115, 22), R.rgb(234, 179, 8),
      R.rgb(16, 185, 129), R.rgb(59, 130, 246), R.rgb(168, 85, 247)
  ];

  let numWaves = 6;
  let waveBaseY = cardY + 12.0 + c6TitleH + (isLandscape ? 16.0 : 24.0);
  let circleRad = isLandscape ? 4.5 : (cardInnerW < 160.0 ? 6.0 : 8.5);
  let maxAvailableWidth = cardInnerW - 16.0;
  let waveSpacing = maxAvailableWidth / (numWaves - 1);
  if (waveSpacing > (isLandscape ? 35.0 : 60.0)) waveSpacing = (isLandscape ? 35.0 : 60.0);

  let totalWaveSpan = waveSpacing * (numWaves - 1);
  let waveStartX = sidePadding + (cardWidth - totalWaveSpan) * 0.5;

  for (let i = 0; i < numWaves; i++) {
    let wx = waveStartX + (i * waveSpacing);
    let waveOffset = Math.sin(g_animTime * 3.5 + (i * 0.6)) * (isLandscape ? 5.0 : 12.0);
    
    if (g_input.mouseDown) {
      let distToTouch = Math.abs(g_input.mouseX - wx);
      if (distToTouch < 45.0) {
        let touchForce = (1.0 - (distToTouch / 45.0)) * 16.0;
        waveOffset += (g_input.mouseY > waveBaseY) ? -touchForce : touchForce;
      }
    }

    let wy = waveBaseY + waveOffset;
    R.circle(wx, wy, circleRad, waveColors[i % 6]);
  }

  // --- CARD 7: SYSTEM DIAGNOSTICS ---
  cardY += chShapes + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chMetrics, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chMetrics, colBorder, 1.0);
  
  let c7CurY = cardY + 12.0;
  c7CurY += R.textWrapped(sidePadding + 12.0, c7CurY, cardInnerW, "System Diagnostics", fTitle, fTitle * 1.25, colTextHead) + 6.0;

  c7CurY += R.textWrapped(sidePadding + 12.0, c7CurY, cardInnerW, diagText1, fSmall, fSmall * 1.35, colTextBody) + 4.0;
  c7CurY += R.textWrapped(sidePadding + 12.0, c7CurY, cardInnerW, diagText2, fSmall, fSmall * 1.35, colTextBody) + 4.0;
  R.textWrapped(sidePadding + 12.0, c7CurY, cardInnerW, diagText3, fSmall, fSmall * 1.35, activeAccent);

  // --- CARD 8: ARCHITECTURE HIGHLIGHTS ---
  cardY += chMetrics + cardSpacing;
  R.rectRounded(sidePadding, cardY, cardWidth, chHighlights, cardRadius, colCard);
  R.outline(sidePadding, cardY, cardWidth, chHighlights, colBorder, 1.0);
  
  let c8CurY = cardY + 12.0;
  c8CurY += R.textWrapped(sidePadding + 12.0, c8CurY, cardInnerW, "RoopM Architecture", fTitle, fTitle * 1.25, colTextHead) + 6.0;

  let lH = fSmall * 1.35;
  c8CurY += R.textWrapped(sidePadding + 12.0, c8CurY, cardInnerW, "+ 100% Software Renderer with Sub-pixel AA", fSmall, lH, colTextBody) + 4.0;
  c8CurY += R.textWrapped(sidePadding + 12.0, c8CurY, cardInnerW, "+ TrueType direct font rasterization", fSmall, lH, colTextBody) + 4.0;
  R.textWrapped(sidePadding + 12.0, c8CurY, cardInnerW, "+ Unified Framebuffer & Cross-Platform Input", fSmall, lH, colTextBody);

  R.pop();

  // ===========================================================================
  // 3. 🌟 ADAPTIVE COLLISION-FREE HEADER
  // ===========================================================================
  R.rect(0, 0, w, headerHeight, R.rgb(15, 23, 42));
  R.line(0, headerHeight, w, headerHeight, R.rgb(30, 41, 59), 1.0);

  let headerContentY = topSafeArea + (isLandscape ? 4.0 : 12.0);

  let logoSize = isLandscape ? 22.0 : (w < 240.0 ? 28.0 : 34.0);
  let logoX = isLandscape ? 10.0 : 12.0;
  R.rectRounded(logoX, headerContentY, logoSize, logoSize, isLandscape ? 4.0 : 8.0, activeAccent);
  R.textCentered(logoX, headerContentY, logoSize, logoSize, "R", isLandscape ? 12.0 : 18.0, '#ffffff');

  let themeBtnW = isLandscape ? 40.0 : (w < 240.0 ? 42.0 : 52.0);
  let themeBtnH = isLandscape ? 16.0 : 24.0;
  let themeBtnX = w - themeBtnW - 12.0;
  let themeBtnY = headerContentY + (isLandscape ? 2.0 : 5.0);

  let textStartX = logoX + logoSize + 8.0;
  let maxHeadTextW = themeBtnX - textStartX - 6.0;
  
  if (maxHeadTextW > 10.0) {
    R.clip(textStartX, 0, maxHeadTextW, headerHeight);
    if (maxHeadTextW < 75.0) {
      R.text(textStartX, headerContentY + 5.0, "RoopM", 14.0, '#ffffff');
    } else {
      R.text(textStartX, headerContentY + (isLandscape ? 0.0 : 2.0), "RoopM Engine", isLandscape ? 10.5 : 15.5, '#ffffff');
      R.text(textStartX, headerContentY + (isLandscape ? 12.0 : 20.0), "Pure C++ UI", isLandscape ? 6.5 : 10.0, R.rgb(148, 163, 184));
    }
    R.pop();
  }

  if (R.clicked(themeBtnX, themeBtnY, themeBtnW, themeBtnH)) {
    g_isDarkMode = !g_isDarkMode;
  }

  R.rectRounded(themeBtnX, themeBtnY, themeBtnW, themeBtnH, themeBtnH * 0.5, R.rgb(30, 41, 59));
  R.textCentered(themeBtnX, themeBtnY, themeBtnW, themeBtnH, g_isDarkMode ? "Light" : "Dark", isLandscape ? 6.5 : 9.5, R.rgb(244, 246, 249));

  // ===========================================================================
  // 4. FIXED FOOTER
  // ===========================================================================
  let footerY = h - footerHeight;
  R.rect(0, footerY, w, footerHeight, R.rgb(15, 23, 42));
  R.line(0, footerY, w, footerY, R.rgb(30, 41, 59), 1.0);

  let footerTextSize = isLandscape ? 7.0 : 10.5;
  R.clip(sidePadding, footerY, cardWidth, footerHeight);
  R.text(sidePadding, footerY + (isLandscape ? 5.0 : 8.0), "Built with RoopM - One Codebase, Any Platform", footerTextSize, R.rgb(148, 163, 184));
  R.pop();
    // --- C++ Logic End ---

    g_input.scrollDeltaY = 0;
    g_input.pointerJustPressed = false;
    g_input.pointerJustReleased = false;
    requestAnimationFrame(update);
}
update();
