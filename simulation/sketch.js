let backgroundColor = 0;
let textColor = 255-backgroundColor;


let img;
let cassette;

let speedSlider, inputIntervalSlider;
let speedSliderLabel, inputIntervalSliderLabel;
let frameButton,borderButton;
let startButton, manualButton, halfFullButton, unlockButton, concertButton;
let lastTick = 0;
let lastInputTick = 0;

const r = 12;
const PIXEL_COLOR_PER_INPUT = 2; // number of each color pixel in 1 input chunk

let lightOff, colorFinal;
let colorInput = [];

let timeoutLeft, timeoutRight;

function preload() {
  img = loadImage("cassette-full.png");
}

function setup() {
  serialSetup(); // serial.js file
  createCanvas(windowWidth, windowHeight);

  lightOff = color("#b8b8b8");
  lightOff.setAlpha(32);
  colorFinal = color("#894CE0");
  colorInput = ["#8D7EFC", "#BE94FF", "#CD6EFF", "#FFA9FF", "#FFF"];

  cassette = new Cassette(width/2-300, 90, 600, 376);

  
  speedSliderLabel = createP("Speed");
  speedSliderLabel.style('color', `rgb(${textColor},${textColor},${textColor})`);
  speedSliderLabel.position(10, height-60);
  speedSlider = createSlider(5, 200, 80, 10);
  speedSlider.position(50, height-60);
 

  // inputIntervalSlider = createSlider(1, 20, 5, 1);
  // inputIntervalSliderLabel = createP("Input interval");
  // inputIntervalSliderLabel.position(140, height + 20);

  textAlign(LEFT, TOP);
  textSize(16);

  frameButton = createButton("Toggle Frame");
  frameButton.mousePressed(() => cassette.switchFrameDisplay());
  frameButton.position(10, 60);

  borderButton = createButton("Toggle Border");
  borderButton.mousePressed(() => cassette.switchBorderDisplay());
  borderButton.position(120, 60);

  manualButton = createButton("Manual");
  manualButton.mousePressed(() => cassette.switchState("manual"));
  manualButton.position(10, 90);

  startButton = createButton("Start");
  startButton.mousePressed(() => cassette.switchState("start"));
  startButton.position(10, 120);

  halfFullButton = createButton("Half Full");
  halfFullButton.mousePressed(() => cassette.switchState("halfFull"));
  halfFullButton.position(10, 150);

  unlockButton = createButton("Unlock");
  unlockButton.mousePressed(() => cassette.switchState("unlock"));
  unlockButton.position(10, 180);

  concertButton = createButton("Concert");
  concertButton.mousePressed(() => cassette.switchState("concert"));
  concertButton.position(10, 210);

  if (cassette.state === "start" || cassette.state === "halfFull") {
    scheduleInputLeft();
    scheduleInputRight();
  }
}

function draw() {
  background(0);
  fill(textColor);

  if (cassette.state === "manual") {
    text("Press 'Q' to add joy pixels (left), 'P' to add joy pixels (right)", 10, 10);
  }

  text("'W' → unlocking mode, and 'E' → manual control mode.", 10, 30);
  text(`Current state: ${cassette.state}`, 10, height-20);


  if (millis() - lastTick >= speedSlider.value()) {
    cassette.tick();
    lastTick = millis();
  }
  cassette.draw();
}

function keyPressed() {
  if (cassette.state === "manual") {
    switch (key) {
      case "p":
        cassette.legRight.addPixels();
        break;
      case "q":
        cassette.legLeft.addPixels();
        break;
    }
  }
  switch (key) {
    case "w":
      cassette.switchState("unlock");
      break;
    case "e":
      cassette.switchState("start");
      cassette.switchState("manual");
      break;
  }
}

function scheduleInputLeft() {
  let interval = random(5000, 15000);
  timeoutLeft = setTimeout(() => {
    cassette.legLeft.addPixels();
    scheduleInputLeft();
  }, interval);
}

function scheduleInputRight() {
  let interval = random(5000, 15000);
  timeoutRight = setTimeout(() => {
    cassette.legRight.addPixels();
    scheduleInputRight();
  }, interval);
}

class Cassette {
  constructor(x, y, w, h) {
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
    this.state = "manual";
    this.showFrame = true;
    this.showBorder = true;

    this.borderPixels = this.generateBorderPixels();
    this.borderColors = this.borderPixels.map((_) => colorFinal);

    let spoolY = y + h / 2;
    let spoolX1 = x + 165;
    let spoolX2 = x + 435;
    this.spoolLeft = new Spool(spoolX1, spoolY, this.y + this.h, this.state);
    this.spoolRight = new Spool(spoolX2, spoolY, this.y + this.h, this.state);

    this.infinityLoop = new InfinityLoop(spoolX1, spoolX2, spoolY);

    this.legLeft = new Leg(
      spoolX1,
      this.y + this.h,
      445, // height of leg
      this.addPixelsLeft.bind(this)
    );
    this.legRight = new Leg(
      spoolX2,
      this.y + this.h,
      445, // height of leg
      this.addPixelsRight.bind(this)
    );
  }

  switchState(newState) {
    this.state = newState;
    this.spoolLeft.switchState(newState);
    this.spoolRight.switchState(newState);

    clearTimeout(timeoutLeft);
    clearTimeout(timeoutRight);
    if (newState === "start" || newState === "halfFull") {
      scheduleInputLeft();
      scheduleInputRight();
    }
  }

  switchFrameDisplay() {
    this.showFrame = !this.showFrame;
  }

  switchBorderDisplay() {
    this.showBorder = !this.showBorder;
  }

  addPixelsLeft(c) {
    this.spoolLeft.addPixels(c);
  }

  addPixelsRight(c) {
    this.spoolRight.addPixels(c);
  }

  tick() {
    // BORDER ANIMATION
    // this.i = (this.i + 1) % this.borderPixels.length;
    // this.borderColors.unshift(this.borderColors.pop());

    // if (this.queue > 0 && this.i % 4 === 0) {
    //   this.queue -= 4;
    //   for (let j = 0; j < 4; j++) {
    //     this.borderColors[j] = colorFinal;
    //   }
    // }

    this.legLeft.tick();
    this.legRight.tick();
    this.spoolLeft.tick();
    this.spoolRight.tick();

    this.infinityLoop.tick();
  }

  draw() {
    this.legLeft.draw();
    this.legRight.draw();

    this.spoolLeft.draw();
    this.spoolRight.draw();

    if(this.spoolLeft.pixelsCounter > this.spoolLeft.pixelsPerCircle * this.spoolLeft.maxCircles 
      && this.spoolRight.pixelsCounter > this.spoolRight.pixelsPerCircle * this.spoolRight.maxCircles
    ) {
      this.switchState("unlock");
    }

    if(this.showBorder){
      this.borderPixels.forEach((pixel, i) => {
        pixel.setColor(this.borderColors[i]);
        pixel.draw();
      });
    }
 
    if(this.showFrame){
      image(img, this.x, this.y, this.w,this.w*img.height/img.width);
    }

    this.infinityLoop.switchState(this.state);
    this.infinityLoop.draw();
  }

  generateBorderPixels() {
    const p = [];

    let m = 12; // margin
    // bottom row from right
    for (let i = this.x + this.w - m; i > this.x + m; i -= r) {
      p.push(new Pixel(i, this.y + this.h - m));
    }
    return p;
  }
}

class InfinityLoop {
  constructor(spoolLeftX, spoolRightX, centerY) {
    this.centerX = (spoolLeftX + spoolRightX) / 2;
    this.centerY = centerY;
    this.width = (spoolRightX - spoolLeftX) *.95;
    this.state = "manual";

    this.numPoints = 60;
    this.t = 0;

    this.pixels = this.generatePixels();
    this.pixelColors = this.pixels.map(() => lightOff);

    // Parameters for moving yellow pixels
    this.activePixels = 6;
    this.currentIndex = 0;
  }

  generatePixels() {
    const pixels = [];
    const step = TWO_PI / this.numPoints;

    for (let t = 0; t < TWO_PI; t += step) {
      // Parametric equations for infinity curve (lemniscate)
      let x = this.centerX + (this.width * cos(t)) / (1 + sin(t) * sin(t));
      let y =
        this.centerY +
        (this.width * sin(t) * cos(t)) / (1 + (sin(t) * sin(t)) / TWO_PI);

      pixels.push(new Pixel(x, y));
    }

    return pixels;
  }

  switchState(newState) {
    this.state = newState;
  }

  tick() {
    if (this.state === "unlock" || this.state === "concert") {
      // Update pixel colors
      this.pixelColors = this.pixels.map((_, i) => {
        let distance =
          (i - this.currentIndex + this.numPoints) % this.numPoints;
        if (distance < this.activePixels) {
          let intensity = map(distance, 0, this.activePixels - 1, 64, 255);
          let c = color("yellow");
          c.setAlpha(intensity);
          return c;
        }
        return lightOff;
      });

      // Move the active section
      this.currentIndex = (this.currentIndex + 1) % this.numPoints;
    } else {
      this.pixelColors = this.pixels.map(() => lightOff);
    }
  }

  draw() {
    // Draw each pixel with its glow effect
    this.pixels.forEach((pixel, i) => {
      let c = this.pixelColors[i];

      if (c !== lightOff) {
        // Draw glow effect for active pixels
        for (let j = 2; j >= 0; j--) {
          let glowColor = color("yellow");
          glowColor.setAlpha(alpha(c) / (j + 2));
          fill(glowColor);
          circle(pixel.x, pixel.y, r * (j + 1.5));
        }
      }

      // Draw the main pixel
      pixel.setColor(c);
      pixel.draw();
    });
  }
}

class Leg {
  constructor(cx, yTop, h, edgeCallback) {
    this.cx = cx;
    this.yTop = yTop;
    this.h = h;

    this.pixels = this.generatePixels();
    this.pixelColors = this.pixels.map((_) => lightOff);

    this.edgeCallback = edgeCallback;
    this.i = 0;
    this.queue = 0;
  }

  addPixels() {
    this.queue += colorInput.length * PIXEL_COLOR_PER_INPUT; // pixels per input
  }

  tick() {
    if (this.queue > 0) {
      this.queue--;

      // 2 pixels in a row of each color per input
      const c = colorInput[Math.floor(this.i / PIXEL_COLOR_PER_INPUT)];
      this.pixelColors.unshift(color(c));
      this.i = (this.i + 1) % (colorInput.length * PIXEL_COLOR_PER_INPUT);
    } else {
      this.pixelColors.unshift(lightOff);
    }

    // last pixel should move up onto spool
    const p = this.pixelColors.pop();
    this.edgeCallback(p);
  }

  draw() {
    this.pixels.forEach((pixel, i) => {
      pixel.setColor(this.pixelColors[i]);
      pixel.draw();
    });
  }

  generatePixels() {
    const p = [];
    for (let y = this.yTop + this.h; y > this.yTop; y -= r) {
      p.push(new Pixel(this.cx, y));
    }
    return p;
  }
}

class Spool {
  constructor(x, y, borderBottom, state) {
    this.x = x;
    this.y = y;
    this.cassetteBorderBottom = borderBottom;
    this.state = state;

    // spiral parameters
    // lmao magic numbers
    this.initialAngle = 1.13;
    this.angle = this.initialAngle;
    this.initialRadius = 43;
    this.radius = this.initialRadius;
    this.spacing = 10.02;
    this.numPixels = 280;

    this.pixels = this.generatePixels();
    this.pixelColors = this.pixels.map((_) => lightOff);

    this.queue = [];

    this.pixelsCounter = 0;
    this.pixelsPerCircle = 3 * PIXEL_COLOR_PER_INPUT * colorInput.length;
    this.maxCircles = 6;

    this.rotating = false;
    this.rotateAngle = 0;
  }

  switchState(newState) {
    this.state = newState;

    if (newState === "start") {
      this.pixelsCounter = 0;
    } else if (newState === "halfFull") {
      this.pixelsCounter = floor((this.pixelsPerCircle * this.maxCircles) / 2);
    } else if (newState === "unlock" || newState === "concert") {
      this.pixelsCounter = this.pixelsPerCircle * this.maxCircles;
    }
    this.rotating = newState === "unlock" || newState === "concert";

    this.updateCircles();
  }

  addPixels(c) {
    this.queue.push(c);
  }

  tick() {
    if (this.queue.length > 0) {
      let c = this.queue.shift();
      this.pixelColors.unshift(c);
    } else {
      this.pixelColors.unshift(lightOff);
    }

    if (this.rotating) {
      this.rotate();
    }
  }

  draw() {
    // incoming pixels moving towards center
    this.pixels.forEach((pixel, i) => {
      pixel.setColor(this.pixelColors[i]);
      pixel.draw();
    });

    this.updateCircles();

    // filled rings
    let filledRings = floor(this.pixelsCounter / this.pixelsPerCircle);
    let pixelsInCurrentRing = this.pixelsCounter % this.pixelsPerCircle;

    this.pixels.forEach((pixel, i) => {
      // Turn off pixels in current rotation angle
      if (this.rotating) {
        let theta = atan2(pixel.y - this.y, pixel.x - this.x);
        theta = (theta + TWO_PI) % TWO_PI;
        if (abs(theta - this.rotateAngle) < 0.2 && pixel.ringNumber > 0) {
          pixel.setColor(lightOff);
          pixel.draw();
          return;
        }
      }

      if (pixel.ringNumber <= filledRings && pixel.ringNumber > 0) {
        // Rings that are fully filled
        pixel.setColor(colorFinal);
        pixel.draw();
      } else if (pixel.ringNumber === filledRings + 1) {
        // Current ring that is partially filled
        let opacity = map(pixelsInCurrentRing, 0, this.pixelsPerCircle, 0, 255);
        pixel.setColor(
          color(red(colorFinal), green(colorFinal), blue(colorFinal), opacity)
        );
        pixel.draw();
      }
    });
  }

  updateCircles() {
    // Check if the last pixel in the array has reached the center of the spool
    let maxPixels = this.pixelsPerCircle * this.maxCircles;
    if (
      this.pixels[this.pixels.length - 1].color !== lightOff &&
      this.pixelsCounter < maxPixels
    ) {
      // if it has color, increment the counter
      this.pixelsCounter++;
    }
  }

  rotate() {
    this.rotateAngle = (this.rotateAngle + PI / 24) % TWO_PI;
  }

  generatePixels() {
    const p = [];
    let cx, cy;
    let currentRing = 1;

    for (let i = 0; i < this.numPixels; i++) {
      cx = this.x + this.radius * cos(this.angle);
      cy = this.y + this.radius * sin(this.angle);

      p.push(new Pixel(cx, cy, currentRing));

      // maintain constant distance between pixels like on LED strip
      let deltaAngle = this.spacing / this.radius;
      this.angle += deltaAngle;
      this.radius += (this.spacing * deltaAngle) / TWO_PI;

      if (this.angle >= TWO_PI + this.initialAngle) {
        this.angle -= TWO_PI;
        currentRing++;
      }
    }

    // from spool to border bottom
    const numLegPixels = Math.floor((this.cassetteBorderBottom - cy) / r);
    for (let i = 0; i < numLegPixels; i++) {
      let y = cy + r * (i + 1);
      p.push(new Pixel(cx, y));
    }
    return p.reverse();
  }
}

class Pixel {
  constructor(x, y, ringNumber = 0) {
    this.x = x;
    this.y = y;
    this.color = lightOff;
    this.ringNumber = ringNumber;
  }

  setColor(c) {
    this.color = c;
  }

  draw() {
    noStroke();
    fill(this.color);
    circle(this.x, this.y, r);
  }
}
