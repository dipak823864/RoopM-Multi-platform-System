```
Act as a Principal C++ Systems and UI Engine Architect. Design and construct an ultra-complete, production-grade, multi-platform UI Engine and Layout Framework in Pure Modern C++ (C++20 standard) targeting Desktop (Windows, macOS, Linux) and Mobile (Android, iOS). You must systematically implement every single subsystem, micro-interaction, control, widget, state, gesture, and drag-and-drop mechanism explicitly defined below without abstracting or skipping any feature.

### SYSTEM 1: EXHAUSTIVE DRAG, DROP & POINTER MANIPULATION MECHANICS
- Drag-and-Drop Infrastructure:
  - `DragSource`: Initiates drag on mouse threshold (pixels moved) or long-press touch timeout.
  - `DragPayload`: Multi-format data container (Plain Text, Unicode, Custom Binary, File Paths, Image Data).
  - `DragVisual` / `DragGhost`: Semi-transparent floating preview following cursor/touch centroid with offset adjustment.
  - `DropTarget`: Receiving node implementing `OnDragEnter`, `OnDragOver`, `OnDragLeave`, `OnDrop`, and `OnDragCancel` (via Escape key or touch cancel).
- Specific Drag Use-Cases to Implement:
  1. Window Manipulation Drag: Frameless window dragging, caption dragging, multi-edge and 4-corner window resizing.
  2. Canvas & Viewport Panning Drag: 2D infinite canvas translation via Middle-Mouse or Space+Left-Click or Multi-finger touch drag.
  3. Scrollbar Thumb Dragging: Linear tracking with acceleration and jump-to-click functionality.
  4. Slider & Range-Slider Thumb Dragging: Horizontal/Vertical track dragging with step-snapping and dual-thumb range selection.
  5. Splitter & Pane Resizing Drag: Live split-ratio adjustment between horizontal/vertical panels with min/max constraint limits.
  6. Text Selection Drag: Precise character-level selection drag, word-by-word drag on double-click-hold, and paragraph drag on triple-click-hold.
  7. Reorderable List/Grid Drag: Dynamic item dragging with animated layout displacement of sibling items (Kanban card reordering, Tab reordering).
  8. Color Wheel & Palette Drag: 2D Saturation-Value square dragging and 1D Hue ring dragging.
  9. Native OS Drag-and-Drop: Receiving external dragged files/folders from Desktop Explorer/Finder directly into UI nodes.
  10. Swipe-to-Dismiss Drag: Horizontal swipe on list items to reveal background actions (Delete, Archive) with snap-back or fling-out thresholds.
  11. Pull-to-Refresh Drag: Vertical overscroll dragging with elastic tension resistance, threshold trigger, and loading spinner reveal.
  12. Docking & Undocking Drag: Detaching tool panels into separate OS floating windows and re-docking via drop-zone visual overlays (Top, Bottom, Left, Right, Center).

### SYSTEM 2: COMPLETE GESTURE RECOGNITION & GESTURE ARENA (CONFLICT RESOLUTION)
- Gesture Arena / Ambiguity Resolver: Disambiguates competing gestures (e.g., resolving whether a vertical drag is a ScrollView scroll, a Slider drag, or a Tap).
- Pointer Recognizers:
  - `TapRecognizer`: Single tap, Double tap, Triple tap with configurable max distance and timeout intervals.
  - `LongPressRecognizer`: Press-and-hold detection with configurable trigger duration and continuous hold tick events.
  - `PanGestureRecognizer`: Multi-directional drag tracking with instantaneous velocity calculation.
  - `Fling/SwipeRecognizer`: Directional flick tracking (Up, Down, Left, Right) with momentum-based inertia physics.
  - `PinchToZoomRecognizer`: Multi-touch scale factor calculation relative to dynamic touch focal/pivot point.
  - `RotationGestureRecognizer`: Two-finger angular rotation tracking in radians.
  - `EdgeSwipeRecognizer`: Screen boundary swipe detection for opening off-canvas drawers.
- Multi-Touch Engine: Unique Pointer ID tracking (simultaneous tracking for up to 10 independent fingers).

### SYSTEM 3: COMPREHENSIVE WIDGET STATE MATRIX
Every interactive widget must natively support and visually transition between the following discrete states:
1. `Normal` / `Default` (Idle state)
2. `Hovered` / `PointerOver` (Mouse hover or stylus proximity)
3. `Pressed` / `Active` (Mouse down or active touch contact)
4. `Focused` (Keyboard focus ring active)
5. `FocusVisible` (Focus ring visible strictly on keyboard navigation, hidden on mouse click)
6. `Disabled` (Non-interactive, visual gray-out, events swallowed)
7. `Checked` / `Unchecked` / `Indeterminate` (For toggleables)
8. `Dragging` (Widget is the active drag source)
9. `DragOver` (Widget is hovered by an active drag payload)
10. `Selected` (For list items, tabs, and data rows)
11. `Loading` / `Busy` (Interaction disabled, showing local spinner)
12. `Invalid` / `Error` (Form validation failure state with red outline)

### SYSTEM 4: EXHAUSTIVE UI CONTROL & WIDGET INVENTORY

- Core Text & Typography:
  - `Label` / `TextBlock`: Multi-line text with wrapping, truncation, ellipsis ("...").
  - `RichText`: Spans with mixed fonts, weights, colors, inline links, and embedded inline widgets.
  - `SelectableText`: Selectable text with mobile-style selection teardrop handles and copy popup.

- Buttons & Indicators:
  - `Button`: Standard button with text and icon slots.
  - `IconButton`: Circular or square icon button with ripple effect.
  - `ToggleButton` & `SegmentedButton`: Single and multi-select button groups.
  - `SplitButton`: Primary action button with an attached drop-down trigger button.
  - `FloatingActionButton`: Circular floating action button for primary mobile actions.
  - `Badge`: Numerical or dot indicator overlay attached to corners of icons/avatars.

- Form & Input Controls:
  - `TextField` / `TextBox`: Single-line text input with clear button, prefix/suffix icons, selection caret, and IME support.
  - `TextArea`: Multi-line text input with automatic height expansion and line numbering support.
  - `PasswordField`: Masked text input with eye-icon visibility toggle.
  - `NumberInput` / `NumericStepper`: Number input with increment/decrement up-down spin buttons.
  - `Checkbox`: Square check control supporting tri-state logic.
  - `RadioButton` & `RadioGroup`: Mutually exclusive circular selection items.
  - `Switch` / `Toggle`: iOS/Material style sliding switch with smooth thumb animation.
  - `Slider`: Single thumb horizontal/vertical value slider with value tooltip popup.
  - `RangeSlider`: Dual-thumb slider for selecting minimum and maximum numeric ranges.
  - `Dropdown` / `ComboBox`: Single-select dropdown with auto-filtering search box.
  - `MultiSelectDropdown`: Dropdown allowing multiple item selections with tag-chips.
  - `DatePicker`: Calendar grid picker with month/year navigation, single date, and date-range selection.
  - `TimePicker`: Analog clock dial and digital spinner time selection widget.
  - `ColorPicker`: Full HSV/RGB/HEX color picker with 2D palette box, alpha slider, and palette swatches.
  - `Chip` / `TagInput`: Removable text tags with input field for adding new chips.

- Containers, Panels & Structural Controls:
  - `Container` / `Box`: Basic layout building block with background, borders, and margins.
  - `Card`: Elevated structural panel with drop shadows and rounded corners.
  - `Accordion` / `Expander`: Collapsible disclosure panel with smooth expanding animation.
  - `SplitterView`: Two or three-pane layout with draggable divider bars.
  - `DockingHost`: Advanced docking canvas supporting detachable floating panels and tabbed dock groups.
  - `Divider`: 1px horizontal and vertical separator lines with optional text label in center.

- Scrolling & Collection Presenters:
  - `ScrollView`: 1D and 2D scrollable viewport with kinetic scrolling, friction physics, and rubber-band bounce.
  - `ScrollBar`: Desktop-style draggable bar and mobile-style auto-fading scroll pill.
  - `ListView`: Recycler list engine (virtualized rendering for millions of items with dynamic heights).
  - `GridView`: Virtualized uniform and auto-fit multi-column grid.
  - `MasonryGrid`: Pinterest-style staggered height virtualized grid.
  - `PageView` / `Carousel`: Full-width swiping page container with dot indicators and auto-play timer.
  - `TreeView`: Hierarchical tree with expanding/collapsing parent nodes, node selection, and indentation guides.
  - `DataTable`: Spreadsheet-like grid with sortable headers, resizable columns, row selection, and frozen columns.

- Overlays, Popups & Dialogs:
  - `ModalDialog`: Center-screen blocking dialog with title, body, action buttons, and backdrop blur.
  - `AlertDialog`: Standardized confirmation prompt (OK/Cancel, Yes/No/Cancel).
  - `Drawer` / `SideSheet`: Off-canvas panel sliding from Left, Right, Top, or Bottom.
  - `Tooltip`: Small floating text label appearing on hover or long-press.
  - `Popover`: Rich contextual container positioned relative to an anchor widget.
  - `ContextMenu`: Desktop right-click popup menu with nested submenus, icons, shortcuts, and separators.
  - `Toast` / `SnackBar`: Auto-dismissing floating alert banner with action button and duration timer.

- Navigation & Shell Controls:
  - `WindowFrame` / `TitleBar`: Custom dark/light window caption bar with Min, Max, Close, and Icon.
  - `MenuBar`: Desktop top application menu bar (File, Edit, View, Help) with cascading flyouts.
  - `ToolBar`: Icon action bar with overflow button handling.
  - `TabBar` & `TabControl`: Top/Bottom tab strip with animated sliding selection indicator.
  - `NavigationBar`: Mobile bottom navigation bar with icons and labels.
  - `Breadcrumb`: Hierarchical navigation path trail with clickable segment links.
  - `Pagination`: Page navigation bar with Previous, Next, and Page number buttons.

- Progress & Status Indicators:
  - `LinearProgressBar`: Determinate (0-100%) and Indeterminate animated pulsing loading bar.
  - `CircularProgressIndicator` / `Spinner`: Rotating circular loader.
  - `SkeletonLoader`: Shimmering placeholder box simulating content loading.

### SYSTEM 5: LAYOUT ENGINE (MATHEMATICAL MODEL)
- Layout Algorithms:
  - `Flexbox`: Row, Column, Expanded, Flexible, Spacer, Wrap with complete CrossAxisAlignment and MainAxisAlignment.
  - `Grid`: Fixed and fractional (`fr`) columns/rows, Auto-placement, RowSpan, ColSpan, Gap (RowGap, ColumnGap).
  - `Stack`: Layered Z-index positioning, Positioned offsets (Left, Top, Right, Bottom), Alignment pins.
  - `AnchorLayout`: Docking edges to parent boundaries.
  - `AspectRatio`: Forcing strict width-to-height proportions (e.g., 16:9, 1:1, 4:3).
  - `ConstraintEngine`: Enforcing MinWidth, MaxWidth, MinHeight, MaxHeight, and Intrinsics (MinIntrinsicWidth, MaxIntrinsicWidth).

### SYSTEM 6: ADVANCED VISUAL STYLING, GRAPHICS & SHADERS
- Geometry & Borders:
  - Independent 4-Corner Radius: `TopLeft`, `TopRight`, `BottomLeft`, `BottomRight` with elliptical support.
  - Border Strokes: Solid, Dashed, Dotted, Inner/Center/Outer alignment, Gradient border strokes.
  - 9-Slice Scaling: Bitmap 9-slice image borders (resizable dialog frames without corner distortion).
- Shadows & Lighting:
  - Multi-layered Outer Drop Shadows (Blur, Spread, Offset X/Y, Color).
  - Multi-layered Inner Shadows (Inset shadows).
- Shaders & Filters:
  - Color Blend Modes: Normal, Multiply, Screen, Overlay, Darken, Lighten, ColorDodge, ColorBurn.
  - Backdrop Blur (Frosted glass / Acrylic / Glassmorphism effect).
  - Signed Distance Field (SDF) rendering for crisp vector shapes and ultra-sharp rounded corners.
  - GPU Scissor & Stencil Masking for strict non-rectangular clipping.

### SYSTEM 7: LOW-LEVEL PLATFORM, RENDERING & PERFORMANCE INTERNALS
- Platform Abstraction Layer:
  - Cross-platform native windowing, system cursor shapes, multi-monitor HiDPI auto-scaling.
  - Full IME (Input Method Editor) integration for composition strings, candidate windows, and Indic/CJK text.
  - Native system tray integration, system notifications, and global hotkey bindings.
- Typography Core:
  - FreeType + HarfBuzz pipeline for complex script shaping, ligature joining, and bi-directional text (RTL/LTR).
  - Sub-pixel font rasterization (ClearType RGB), Dynamic GPU Glyph Atlas with LRU eviction.
- Rendering & Memory Core:
  - Hardware Abstraction Interface: Vulkan, DirectX 12, Metal, Modern OpenGL.
  - Draw Call Batching: Automatic geometry batching to reduce GPU state changes.
  - Frame Arena Memory Allocator: Zero per-frame heap memory allocations.
  - Zero-CPU Idle Engine: VSync synchronization with immediate thread sleeping when UI is static.
- Diagnostics & Developer Tools:
  - In-Engine Visual Tree Inspector with interactive element hover-picker.
  - Real-time performance HUD: FPS, Frame Duration (ms), CPU Layout Pass time, GPU Draw Calls counter.


Act as a Principal C++ Systems and UI Engine Architect. Design and construct a complete, high-performance, modular, cross-platform UI Engine and Layout Framework in Pure Modern C++ (C++20 standard) optimized for both Desktop (Mouse, Keyboard, Multi-window) and Mobile (Multi-Touch, Gestures, DPI Scaling). Implement every single system, component, controller, and visual feature listed below without omitting any item.

### 1. CORE NAMESPACE & ARCHITECTURAL FOUNDATION
- Engine Root Namespace: `UIEngine::`
- Base Scene Graph Node: `UIElement` / `Widget` with lifecycle hooks: `OnInit()`, `OnUpdate(dt)`, `OnLayout(constraints)`, `OnRender(renderContext)`, `OnEvent(event)`, `OnDestroy()`
- Memory Management: Frame-based Arena Allocator for zero-allocation per-frame UI passes, Object Pool Allocator for UI nodes, and Ref-counted handles for shared resources
- Reactive State Engine: Observable Properties, Signal/Slot (Event Dispatcher) mechanism, Dirty Flag Propagation (LayoutDirty, RenderDirty, TransformDirty)
- Universal Unit Resolution: Support for `px`, `dp`, `pt`, `%` (Relative), `vw`, `vh`, `rem`, `em` dynamically converted via `DisplayMetrics`

### 2. PLATFORM ABSTRACTION LAYER (PAL) & SYSTEM INTEGRATION
- Windowing System: Window creation, Fullscreen, Borderless, Minimize, Maximize, Multi-window, Detachable Docking Panels (Win32, Cocoa, Wayland/X11, Android NDK surfaces)
- Power & Lifecycle Manager: Zero-CPU Idle loop (`WaitEvents` when inactive), VSync Frame Pacing (60Hz, 120Hz, 144Hz Variable Refresh Rate), Sleep/Resume state restoration
- Display & DPI Management: HiDPI dynamic scaling factor (100% to 400%), Per-monitor DPI awareness, Pixel Snapping engine (sub-pixel alignment prevention)
- System Clipboard Manager: Async Read/Write for Unicode PlainText, HTML, RTF, and Bitmap/PNG formats
- Native Drag and Drop: OS-to-App and Inter-Widget DragDrop payload system
- Native IME (Input Method Editor) Handler: Support for complex composition, candidate window positioning, dead keys, and non-Latin character input (Indic, CJK)
- Accessibility (a11y) Subsystem: Parallel Semantic Tree generation for Screen Readers (OS Accessible Roles, States, Focus tracking)

### 3. HARDWARE-ACCELERATED RENDERING & COMPOSITOR (RHI)
- Rendering Hardware Interface (RHI): Abstracted graphics backend supporting Vulkan, DirectX 12, Metal, and Modern OpenGL (3.3+)
- Render Command Pipeline: RenderGraph, Deferred Draw Command Buffer with automatic State & Draw Call Batching
- 2D Path Tessellator: Vector Path Engine (Bezier Curves, Arcs, Polygons) tessellated directly to GPU index/vertex buffers
- Layer Compositor: Render-to-Texture Layer Promotion (Offscreen surfaces for caching complex static subtrees), 3D Transform Pipeline via 4x4 Matrix transforms & Perspective Projection
- Tiled Rendering System: Virtual Canvas viewport partition for ultra-large scalable surfaces
- GPU Shader Pipeline: SDF (Signed Distance Field) shape & font rendering, Fast Gaussian Backdrop Blur, Glassmorphism, Rounded Rect Clip shaders
- Clipping & Stencil Masking: Axis-Aligned Scissor Clipping, Arbitrary Vector Path Clipping, Rounded Corner Masking (`ClipToBounds`)

### 4. ADVANCED TYPOGRAPHY & INTERNATIONAL TEXT ENGINE
- Font Loading & Rasterization: FreeType integration for TTF, OTF, and Variable Font axis interpolation (Weight, Width, Slant)
- Complex Text Shaping: HarfBuzz integration for full Indic (Gujarati, Devanagari), Arabic, and complex ligature script handling
- BiDi (Bidirectional) Text Engine: Full Unicode BiDi algorithm for mixed RTL (Right-to-Left) and LTR (Left-to-Right) text layout
- Text Layout & Math: Multi-line wrapping, Knuth-Plass justified line-breaking algorithm, Kerning, Word Spacing, Line Height, Text-Overflow truncation (Ellipsis "...")
- Text Caret & Hit-Testing: Precise (X, Y) coordinate-to-character index resolution, Glyph Bounding Box queries, Bidirectional Arrow-key caret traversal
- Font Atlas & Caching: Dynamic GPU Glyph Texture Atlas with LRU eviction and SDF / Subpixel LCD Anti-Aliasing (ClearType style)
- Font Fallback Chaining: Automated multi-tiered system fallback font resolution for missing glyphs and color emojis

### 5. LAYOUT ENGINE (BOX MODEL & POSITIONING)
- Box Model: `Margin`, `Padding`, `BorderThickness`, `ContentBox`, `BorderBox`
- Constraints Model: `BoxConstraints` (`minWidth`, `maxWidth`, `minHeight`, `maxHeight`, `aspectRatio`)
- Flexbox Layout: `Row`, `Column`, `Flex`, `Expanded`, `Spacer`, `Wrap` (Cross-axis wrapping)
- Flex Alignment Properties: `MainAxisAlignment` (Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly), `CrossAxisAlignment` (Start, Center, End, Stretch, Baseline)
- Grid Layout: Configurable Rows, Columns, Fixed/Fractional (`fr`) sizing, Auto-flow, RowSpan, ColSpan
- Stack & Overlay Layout: Z-Index layer ordering, Positioned Absolute coordinates (Top, Bottom, Left, Right)
- Table Layout: Dynamic cell sizing, Header/Footer pinning, Border collapse rules

### 6. INPUT, EVENT DISPATCH & GESTURE RECOGNITION (DESKTOP + MOBILE)
- Hit-Testing Engine: Deepest-node raycasting with Bounding Box and Non-Rectangular Vector Alpha Hit-Testing
- Unified Pointer Events: `PointerDown`, `PointerUp`, `PointerMove`, `PointerCancel`, `PointerEnter`, `PointerLeave`
- Mouse Controls: `Click`, `DoubleClick`, `TripleClick`, `RightClick` (ContextMenu), `MiddleClick`, `MouseHover`, `MouseWheel` (Vertical/Horizontal Delta), Native Cursor System (Arrow, I-Beam, Hand, ResizeNS, ResizeEW, Forbidden)
- Mobile Touch & Multi-Touch: Multi-finger pointer ID tracking, `Tap`, `DoubleTap`, `LongPress` (with customizable duration), `Pan` (2D drag movement)
- Physics Gestures: `Fling` / `Swipe` with momentum, velocity tracking, and customizable deceleration curves; `PinchToZoom` with centroid tracking; `Rotation` gesture
- Keyboard & Navigation: `KeyDown`, `KeyUp`, `KeyRepeat`, Tab-Focus navigation (`FocusManager`, FocusTrapping for modals, Spatial Navigation for directional pads)
- Micro-Interactions: Haptic Feedback trigger interface and UI Audio trigger dispatchers

### 7. VISUAL STYLING, SHAPES & GEOMETRY
- Geometric Shapes: Rectangle, Circle, Ellipse, Capsule/Pill, Line, Polygon, Custom Path
- Advanced Rounded Corners: Individual 4-corner radii (`topLeft`, `topRight`, `bottomLeft`, `bottomRight`) with elliptical radius support
- Border Styling: Solid, Dashed, Dotted styles, Stroke Alignment (Inside, Center, Outside), Gradient Borders
- Shadow System: `DropShadow` and `InnerShadow` with Blur Radius, Spread Radius, Offset (X, Y), and Color/Alpha
- Color & Paint Engine: Solid RGBA, Linear Gradient (Angle/Stops), Radial Gradient (Center/Radius), Conic/Sweep Gradient
- Visual Effects: Opacity/Alpha blending, Grayscale, Color Inversion, ColorMatrix filters

### 8. COMPLETE UI CONTROLS & WIDGET HIERARCHY

- Typography Widgets:
  - `Label` / `TextBlock`: Plain text display
  - `RichText`: Inline spans with mixed fonts, colors, bold, italic, and click callbacks
  - `SelectableText`: Mouse/Touch drag text selection with copy handles

- Container & Structure Widgets:
  - `Container` / `Box`: Basic stylable element
  - `Card`: Elevated panel with default shadow and rounded corners
  - `Divider` / `Separator`: Horizontal and vertical dividing lines
  - `ScrollView`: 1D and 2D scroll container with kinetic momentum scrolling, elastic overscroll (rubber-banding), and interactive/auto-fading scrollbars
  - `Splitter` / `ResizablePanel`: Draggable splitter bar between split layouts

- Interactive Button Widgets:
  - `Button`: Standard button with Idle, Hover, Pressed, Disabled visual states
  - `IconButton`: Icon-only button with ripple/hover shape
  - `ToggleButton`: Two-state toggle button
  - `FloatingActionButton`: Circular elevated action button

- Form & Data-Entry Widgets:
  - `TextField` / `InputBox`: Single-line text input with placeholder, selection, cursor blinking, clipboard shortcuts, and IME integration
  - `TextArea`: Multi-line text input with word wrap and auto-expanding height
  - `PasswordField`: Masked text input with peek toggle functionality
  - `Checkbox`: Checkable square control with Tri-state (Checked, Unchecked, Indeterminate)
  - `RadioButton` & `RadioGroup`: Mutually exclusive selection controls
  - `Switch` / `ToggleSwitch`: Mobile-style animated sliding switch
  - `Slider` / `RangeSlider`: Continuous and discrete value sliders with single and double thumbs
  - `Dropdown` / `Select`: Expandable single-choice selection list
  - `ColorPicker`: Visual RGB/HSV color picker canvas with hue slider

- Collection & Data Presentation Widgets:
  - `ListView`: High-performance virtualized/recycling 1D list (renders only visible items)
  - `GridView`: High-performance virtualized 2D grid
  - `PageView` / `Carousel`: Swipeable and draggable full-page carousel
  - `TreeView`: Collapsible hierarchical nested node tree
  - `DataTable`: Column-sortable, row-selectable, virtualized data grid with pinned headers

- Feedback, Overlays & Indicators:
  - `Modal` / `Dialog`: Center-screen popup with background dimmed barrier
  - `Tooltip`: Hover/Long-press informational floating popup
  - `Toast` / `SnackBar`: Auto-dismissing animated floating notification panel
  - `ContextMenu`: Contextual popup menu with icons, shortcuts, and sub-menus
  - `ProgressBar`: Determinate and indeterminate linear progress bar
  - `ProgressRing` / `Spinner`: Circular rotating loading indicator

- Navigation & Shell Widgets:
  - `AppBar` / `Header`: Top navigation title bar with action slots
  - `NavigationBar` / `TabBar`: Bottom or top tab switching bar with active indicator animations
  - `Drawer` / `Sidebar`: Off-canvas animated sliding navigation drawer

### 9. ANIMATION, PHYSICS & INTERPOLATION ENGINE
- Value Tweening: Interpolation for `float`, `Vector2`, `Vector3`, `Vector4`, `Color`, `Rect`, `Matrix4x4`
- Easing Curves: `Linear`, `EaseInQuad`, `EaseOutQuad`, `EaseInOutCubic`, `EaseInExpo`, `EaseOutElastic`, `EaseOutBounce`
- Physics Simulation: Spring Simulation (`stiffness`, `damping`, `mass`), Gravity, Friction/Deceleration curves
- Animation Controller: `Play()`, `Pause()`, `Reverse()`, `Seek()`, `Loop()`, `OnFinish()`
- Transition Primitives: `FadeTransition`, `SlideTransition`, `ScaleTransition`, `SizeTransition`

### 10. DIAGNOSTICS, THEME & DEVELOPER TOOLS
- In-Engine Visual Inspector: Runtime visual UI element picker, Box-model overlay (Margin/Padding visualizer), Dynamic property editor
- Performance Profiler Overlay: Real-time graphs for FPS, Frame-time (ms), CPU Layout Pass time, GPU Draw Calls counter, Vertex/Index memory counters
- Dynamic Theme Engine: Centralized Design Tokens (Color Palettes, Typography scales, Spacing constants) with instant runtime Light/Dark Theme hot-swapping
```