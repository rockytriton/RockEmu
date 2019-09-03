//
//  AppDelegate.swift
//  RockEmu
//
//  Created by Rocky Pulley on 8/29/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

import Cocoa

import IOKit
import IOKit.usb
import IOKit.hid

let runloop : CFRunLoop = CFRunLoopGetCurrent()

var valueCallback : IOHIDValueCallback = {
    (context, result, sender, value) in
    
    let element = IOHIDValueGetElement(value)
    let cookie = IOHIDElementGetCookie(element)
    let code = IOHIDValueGetIntegerValue(value)
    
    var event : CGEvent?
    
    switch cookie {
    case 7:     /* Button 1 */
        print("Button 1: ", code)
    //event = generateMediaKeyEvent(key: NX_KEYTYPE_SOUND_DOWN, down: code == 1)
    case 8:     /* Button 2 */
        print("Button 2: ", code)
    //event = generateMediaKeyEvent(key: NX_KEYTYPE_PREVIOUS, down: code == 1)
    case 9:     /* Button 3 */
        print("Button 3: ", code)
    //event = generateMediaKeyEvent(key: NX_KEYTYPE_PLAY, down: code == 1)
    case 10:     /* Button 4 */
        print("Button 4: ", code)
    //event = generateMediaKeyEvent(key: NX_KEYTYPE_NEXT, down: code == 1)
    case 11:     /* Button 5 */
        print("Button 5: ", code)
    //event = generateMediaKeyEvent(key: NX_KEYTYPE_SOUND_UP, down: code == 1)
    case 16:    /* Jog Dial */
        print("Jog Dial: ", code)
        event = nil
    case 17:    /* Wheel */
        print("Wheel   : ", code)
        event = nil
    default:
        print("Unknown element")
        event = nil
    }
    
    if (event != nil) {
        event!.post(tap:.cgSessionEventTap)
    }
}

var devattachCallback : IOHIDCallback = {
    (context, result, sender) in
    print("DEV ATT")
}

let vendorKey = kIOHIDVendorIDKey as CFString
let productKey = kIOHIDProductIDKey as CFString

typealias HIDUsage = UInt32

class Gamepad {
    var device : IOHIDDevice
    
    var buttons : [HIDUsage : Bool] = [:]
    var continuous : [HIDUsage : Float] = [:]
    
    var x   : Float { return continuous[HIDUsage(kHIDUsage_GD_X)] ?? 0.0 }
    var y   : Float { return continuous[HIDUsage(kHIDUsage_GD_Y)] ?? 0.0 }
    var z   : Float { return continuous[HIDUsage(kHIDUsage_GD_Z)] ?? 0.0 }
    var rx  : Float { return continuous[HIDUsage(kHIDUsage_GD_Rx)] ?? 0.0 }
    var ry  : Float { return continuous[HIDUsage(kHIDUsage_GD_Ry)] ?? 0.0 }
    var rz  : Float { return continuous[HIDUsage(kHIDUsage_GD_Rz)] ?? 0.0 }
    var hat : Float { return continuous[HIDUsage(kHIDUsage_GD_Hatswitch)] ?? 0.0 }
    
    init(_ newDevice: IOHIDDevice) {
        device = newDevice
    }
}

let MAX_CONTROLLERS = 4
let controllers = Controllers()

class Controllers {
    var gamepads = [Gamepad?](repeating: nil, count: MAX_CONTROLLERS)
    //var keyboard = Keyboard()
}

func gamepadEvent(_ context: UnsafeMutableRawPointer?, _ result: IOReturn, _ sender: UnsafeMutableRawPointer?, _ value: IOHIDValue) {
    print("EVT")
    let gamepad = context!.bindMemory(to: Gamepad.self, capacity: 1).pointee
    let element = IOHIDValueGetElement(value)
    let elementType = IOHIDElementGetType(element)
    let usage = IOHIDElementGetUsage(element)
    
    print("USAGE: ", usage, " VAL: ", IOHIDValueGetIntegerValue(value))
    
    if elementType == kIOHIDElementTypeInput_Button {
        let pressed = (IOHIDValueGetIntegerValue(value) != 0)
        gamepad.buttons[usage] = pressed
    }
    else if elementType == kIOHIDElementTypeInput_Axis || elementType == kIOHIDElementTypeInput_Misc {
        
        
    }
    
    // TODO: Handle hatswitches correctly
    
}

func deviceAdded(_ inContext: UnsafeMutableRawPointer?, inResult: IOReturn, inSender: UnsafeMutableRawPointer?, deviceRef: IOHIDDevice!) {
    
    if IOHIDDeviceConformsTo(deviceRef, UInt32(kHIDPage_GenericDesktop), UInt32(kHIDUsage_GD_GamePad)) || IOHIDDeviceConformsTo(deviceRef, UInt32(kHIDPage_GenericDesktop), UInt32(kHIDUsage_GD_Joystick)) {
        
        print("gamepad added!")
        
        var gamepad = Gamepad(deviceRef)
        
        // Insert gamepad at first available slot,
        // If no slots available, ignore!
        
         var idx = 0
         for gp in controllers.gamepads {
         if gp == nil {
         controllers.gamepads[idx] = gamepad
         break
         }
         idx += 1
         }
        
         IOHIDDeviceRegisterInputValueCallback(deviceRef, gamepadEvent, &controllers.gamepads[idx])
 
    }
    else if IOHIDDeviceConformsTo(deviceRef, UInt32(kHIDPage_GenericDesktop), UInt32(kHIDUsage_GD_Keyboard)) || IOHIDDeviceConformsTo(deviceRef, UInt32(kHIDPage_GenericDesktop), UInt32(kHIDUsage_GD_Keypad)) {
        
        print("keyboard added!")
        
        //IOHIDDeviceRegisterInputValueCallback(deviceRef, keyboardEvent, nil)
        
    }
    else {
        print("unknown device added")
    }
    
}

func deviceRemoved(_ inContext: UnsafeMutableRawPointer?, inResult: IOReturn, inSender: UnsafeMutableRawPointer?, inIOHIDDeviceRef: IOHIDDevice!) {
    
    var gpIdx = 0
    for gp in controllers.gamepads {
        if gp == nil {
            continue
        }
        let gamepad = gp!
        if Unmanaged.passUnretained(inIOHIDDeviceRef!).toOpaque() == Unmanaged.passUnretained(gamepad.device).toOpaque() {
            controllers.gamepads[gpIdx] = nil
            print("gamepad removed!")
            break
        }
        gpIdx += 1
    }
    
    // Unregister value callback
    IOHIDDeviceRegisterInputValueCallback(inIOHIDDeviceRef, nil, nil)
    
}

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    var nesData : NesData?
    
    var worker : DispatchQueue?
    
    
    private func createDeviceMatchingDictionary( usagePage: Int, usage: Int) -> CFMutableDictionary {
        let dict = [
            kIOHIDDeviceUsageKey: usage,
            kIOHIDDeviceUsagePageKey: usagePage
            ] as NSDictionary
        
        return dict.mutableCopy() as! NSMutableDictionary;
    }

    @IBAction func onOpen(_ sender: Any) {
        bus_init();
        
        let dialog = NSOpenPanel();
        
        dialog.title                   = "Choose a .nes file";
        dialog.showsResizeIndicator    = true;
        dialog.showsHiddenFiles        = false;
        dialog.canChooseDirectories    = true;
        dialog.canCreateDirectories    = true;
        dialog.allowsMultipleSelection = false;
        dialog.allowedFileTypes        = ["nes"];
        
        if (dialog.runModal() == NSApplication.ModalResponse.OK) {
            let result = dialog.url // Pathname of the file
            
            if (result != nil) {
                let path = result!.path
                
                let alert = NSAlert()
                alert.messageText = path
                alert.informativeText = "Game"
                alert.alertStyle = NSAlert.Style.warning
                alert.addButton(withTitle: "OK")
                alert.addButton(withTitle: "Cancel")
                //alert.runModal()
                
                //var data = NesData()
                //var address = UnsafeMutableRawPointer(&data)
                
                let data = cpu_open_file(path)
                
                print("OPENING PATH: ", path);
                
                nesData = data?.pointee
                
                NSDocumentController.shared.noteNewRecentDocumentURL(URL(fileURLWithPath:path));
                
                EmuView.drawing = true
            }
        } else {
            // User clicked on "Cancel"
            return
        }
    }
    
    func application(_ sender: NSApplication, openFile filename: String) -> Bool {
        let data = cpu_open_file(filename)
        
        nesData = data?.pointee
        EmuView.drawing = true
        
        return true
    }
    
    
    
    
    func registerCallback(device : IOHIDDevice) {
        
    }
    
    func test() {
        
    }
    
    var attachCallback : IOHIDDeviceCallback = {
        (context, result, sender, device) in
        
        let v = IOHIDDeviceGetProperty(device, vendorKey) as! CFNumber as! Int
        let p = IOHIDDeviceGetProperty(device, productKey) as! CFNumber as! Int
        
        print("Connected")
        var ret = IOHIDDeviceOpen(device, IOOptionBits(kIOHIDOptionsTypeSeizeDevice))
        //IOHIDDeviceRegisterRemovalCallback(device, devattachCallback, nil)
        
        IOHIDDeviceRegisterInputValueCallback(device, gamepadEvent, nil)
        
        if (ret == kIOReturnSuccess) {
            print("SUCCESS")
        }
        
        //IOHIDDeviceScheduleWithRunLoop(device, runloop, CFRunLoopMode.defaultMode!.rawValue)
        
        print(String(format: "attached: vendor %04x device %04x - %d", v, p, ret))
        
        
    }
    
    var detachCallback : IOHIDDeviceCallback = {
        (context, result, sender, device) in
        
        print("Disconnected")
       // let v = IOHIDDeviceGetProperty(device, vendorKey) as! CFNumber as Int
        //let p = IOHIDDeviceGetProperty(device, productKey) as! CFNumber as Int
        
        //print(String(format: "detached: vendor %04x device %04x", v, p))
    }
    
    
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        
        worker = DispatchQueue(label: "cpu");
        worker?.async {
            cpu_run();
        }
        
        var worker2 = DispatchQueue(label: "gp");
        DispatchQueue.main.async {
            
            print("RUNNING LOOP")
            CFRunLoopRun()
            print("RUN DONE")
        }
        
        self.test()
        /*
        let manager = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone));
        let keyboard = createDeviceMatchingDictionary(usagePage: kHIDPage_GenericDesktop, usage: kHIDUsage_GD_GamePad)
        
        IOHIDManagerOpen(manager, IOOptionBits(kIOHIDOptionsTypeNone) )
        IOHIDManagerSetDeviceMatching(manager, keyboard)
        
        let Handle_DeviceMatchingCallback: IOHIDDeviceCallback = {context, result, sender, device in
        }
        let Handle_DeviceRemovalCallback: IOHIDDeviceCallback = {context, result, sender, device in
            print("Disconnected")
        }
        
        let devices = IOHIDManagerCopyDevices(manager)
        
        if (devices != nil) {
            print("Found devices!", devices!)
            let n = CFSetGetCount(devices!)
            let array = UnsafeMutablePointer<UnsafeRawPointer?>.allocate(capacity: n)
            array.initialize(repeating: nil, count: n)
            CFSetGetValues(devices!, array);
            
            IOHIDManagerRegisterDeviceMatchingCallback(manager, deviceAdded, nil)
            IOHIDManagerRegisterDeviceRemovalCallback(manager, detachCallback, nil)
            
            
            IOHIDManagerScheduleWithRunLoop(manager, runloop,
                                            CFRunLoopMode.defaultMode.rawValue)
            IOHIDManagerOpen(manager, IOOptionBits(kIOHIDOptionsTypeNone))
            CFRunLoopRun()
        }
        else {
            print("Did not find any devices :(")
         
         
         
         
         let matchingDictionaries = [
         [
         kIOHIDDeviceUsagePageKey : kHIDPage_GenericDesktop,
         kIOHIDDeviceUsageKey     : kHIDUsage_GD_GamePad,
         ], [
         kIOHIDDeviceUsagePageKey : kHIDPage_GenericDesktop,
         kIOHIDDeviceUsageKey     : kHIDUsage_GD_Joystick,
         ], [
         kIOHIDDeviceUsagePageKey : kHIDPage_GenericDesktop,
         kIOHIDDeviceUsageKey     : kHIDUsage_GD_Keyboard,
         ], [
         kIOHIDDeviceUsagePageKey : kHIDPage_GenericDesktop,
         kIOHIDDeviceUsageKey     : kHIDUsage_GD_Keypad,
         ]
         ]
        }*/
        
        
        let manager = IOHIDManagerCreate(kCFAllocatorDefault, UInt32(kIOHIDOptionsTypeNone))
        
        let matchingDictionaries = [
            [
                kIOHIDDeviceUsagePageKey : kHIDPage_GenericDesktop,
                kIOHIDDeviceUsageKey     : kHIDUsage_GD_GamePad,
            ]
        ]
        IOHIDManagerSetDeviceMatchingMultiple(manager, matchingDictionaries as CFArray)
        IOHIDManagerRegisterDeviceMatchingCallback(manager, deviceAdded, nil)
        IOHIDManagerRegisterDeviceRemovalCallback(manager, deviceRemoved, nil)
        IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), CFRunLoopMode.defaultMode.rawValue)
        IOHIDManagerOpen(manager, UInt32(kIOHIDOptionsTypeNone))
        
        //CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        //CFRunLoopRun()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }


}

