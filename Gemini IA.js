let submenu = require("submenu");
let serial = require("serial");
let keyboard = require("keyboard");
let textbox = require("textbox");
let dialog = require("dialog");
let storage = require("storage");

serial.setup("usart", 115200);

let shouldexit = false;
let path = "/ext/storage.test";

function sendSerialCommand(command, menutype) {
    serial.write(command);
    receiveSerialData(menutype);
}

function receiveSerialData(menutype) {
    textbox.setConfig("end", "text");
    textbox.show();

    serial.readAny(0);

    while (textbox.isOpen()) {
        let rx_data = serial.readAny(250);
        if (rx_data !== undefined) {
            textbox.addText(rx_data);
        }
    }
    serial.write("stop");

    if (menutype === 0) {
        mainMenu();
    }

    if (menutype === 1) {
        startChatting();
    }
}

function promptForText(header, defaultValue) {
    keyboard.setHeader(header);
    return keyboard.text(100, defaultValue, true);
}

function setName() {
    let name = promptForText("Enter your name", "");
    if (name !== undefined) {
        sendSerialCommand(name, 0);
    } else {
        dialog.message("Error", "No name entered.");
        mainMenu();
    }
}

function connectToAP() {
    let apName = promptForText("Enter AP & Password", "");
    if (apName !== undefined) {
        sendSerialCommand(apName, 0);
    } else {
        dialog.message("Error", "No AP name entered.");
        mainMenu();
    }
}

function startChatting() {
    let chatMessage = promptForText("Enter message", "");
    if (chatMessage === undefined) {
        mainMenu();
        return;
    }
    sendSerialCommand(chatMessage, 1);
}

function help() {
    dialog.message("Help", "This is an app to interact with Google Gemini IA using the esp32");
    mainMenu();
}

function mainMenu() {
    submenu.setHeader("Gemini IA");
    submenu.addItem("Set your name", 0);
    submenu.addItem("Connect to AP", 1);
    submenu.addItem("Start Chatting", 2);
    submenu.addItem("Help", 3);

    let result = submenu.show();

    if (result === 0) {
        setName();
    }

    if (result === 1) {
        connectToAP();
    }

    if (result === 2) {
        startChatting();
    }

    if (result === 3) {
        help();
    }

    if (result === undefined) {
        shouldexit = true;
    }
}


function mainLoop() {
    while (!shouldexit) {
        mainMenu();
        let confirm = dialog.message("Exit", "Press OK to exit, Cancel to return.");
        if (confirm === 'OK') {
            sendSerialCommand('stop', 0); 
            break;
        } else {
            
        }
    }
}

function arraybuf_to_string(arraybuf) {
    let string = "";
    let data_view = Uint8Array(arraybuf);
    for (let i = 0; i < data_view.length; i++) {
        string += chr(data_view[i]);
    }
    return string;
}

mainLoop();
