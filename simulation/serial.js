// variable to hold an instance of the p5.webserial library:
const serial = new p5.WebSerial();

// HTML button object:
let portButton;

function serialSetup() {
  // check to see if serial is available:
  if (!navigator.serial) {
    alert("WebSerial is not supported in this browser. Try Chrome or MS Edge.");
  }
   // open whatever port is available:
   serial.on("portavailable", openPort);
  // if serial is available, add connect/disconnect listeners:
  navigator.serial.addEventListener("connect", portConnect);
  navigator.serial.addEventListener("disconnect", portDisconnect);
  // check for any ports that are available:
  serial.getPorts();
  // if there's no port chosen, choose one:
  serial.on("noport", makePortButton);
 
  // handle serial errors:
  serial.on("requesterror", portError);
  // handle any incoming serial data:
  serial.on("data", serialEvent);
  serial.on("close", makePortButton);
}

// if there's no port selected,
// make a port select button appear:
function makePortButton() {
  // create and position a port chooser button:
  portButton = createButton("choose port");
  portButton.position(10, 650);
  // give the port button a mousepressed handler:
  portButton.mousePressed(choosePort);
}

// make the port selector window appear:
function choosePort() {
  serial.requestPort();
}

// open the selected port, and make the port
// button invisible:
//this is the problem
function openPort() {
  // wait for the serial.open promise to return,
  // then call the initiateSerial function
  serial.open({ baudRate: 115200 }).then(initiateSerial);

  // once the port opens, let the user know:
  function initiateSerial() {
    console.log("port open at 115200");
    // serial.write("x");
  }
  // hide the port button once a port is chosen:
  // if (portButton) portButton.hide();
}

// read any incoming data:

function serialEvent() {
  console.log("something") // test if the problem is from resial.readline()
  const inString = serial.readLine();

if (inString !== null) {
  const currentString = inString.trim(); 
  console.log("Received serial data:", currentString); 

  // is this a problematic way to write code? all if
  if (currentString[0] === "p" || currentString[0] === "P") {
    console.log("Action for P");
    cassette.legRight.addPixels();
  } 
  if (currentString[0] === "q" || currentString[0] === "Q") {
    console.log("Action for Q");
    cassette.legLeft.addPixels(); 
  }
  if (currentString[0] === "w" || currentString[0] === "W") {
    console.log("Action for W");
    cassette.switchState("unlock");
  } 
  if (currentString[0] === "e" || currentString[0] === "E") {
    console.log("Action for E");
    cassette.switchState("start");
    cassette.switchState("manual");
  }
} else {
  console.log("inString is null. No data received from serial port."); 
}


}

// pop up an alert if there's a port error:
function portError(err) {
  alert("Serial port error: " + err);
}

// try to connect if a new serial port
// gets added (i.e. plugged in via USB):
function portConnect() {
  console.log("port connected");
  serial.getPorts();
}

// if a port is disconnected:
function portDisconnect() {
  serial.close();
  console.log("port disconnected");
}
