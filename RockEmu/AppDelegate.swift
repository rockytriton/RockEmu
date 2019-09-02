//
//  AppDelegate.swift
//  RockEmu
//
//  Created by Rocky Pulley on 8/29/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    var nesData : NesData?
    
    var worker : DispatchQueue?

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
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
        
        worker = DispatchQueue(label: "cpu");
        worker?.async {
            cpu_run();
        }
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }


}

