//
//  EmuVC.swift
//  RockEmu
//
//  Created by Rocky Pulley on 8/29/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

import Cocoa
import IOKit
import IOKit.usb
import IOKit.hid

class EmuVC: NSViewController {
    @IBOutlet weak var emuImage: NSImageView!
    
    @IBOutlet var emuView: EmuView!
    var rep:NSBitmapImageRep? = nil
    
    var x = 8
    var y = 8
    
    @IBAction func onPauseResume(_ sender: Any) {
        cpu_resume()
    }
    
    @IBAction func onStep(_ sender: Any) {
        
    }
    
    
    @IBAction func onCodeView(_ sender: Any) {
        let newVc = NSWindowController(windowNibName: "CodeViewWC")
        newVc.contentViewController = NSViewController(nibName: "CodeViewVC", bundle: nil)
        newVc.showWindow(sender)
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        emuImage.imageScaling = NSImageScaling.scaleProportionallyUpOrDown
        
        var timer = Timer.scheduledTimer(withTimeInterval: TimeInterval(0.005), repeats: true, block: {_ in self.onDraw()})
        
    }
    
    override func keyDown(with event: NSEvent) {
        if (event.characters!.contains(" ")) {
            print("HIT SPACE");
        }
    }
    override var acceptsFirstResponder: Bool {
        return true
    }
    
    func onDraw() {
        /*
        emuImage.image?.lockFocus()
        
        //rep?.setColor(NSColor(calibratedRed: 1, green: 0, blue: 1, alpha: 1), atX: x, y: y)
        
        var c = NSColor(calibratedRed: 0, green: 1, blue: 1, alpha: 1)
        
        c.drawSwatch(in: NSRect(x: x, y: y, width: 8, height: 8))
        
        x += 10
        y += 10
        
        emuImage.image?.unlockFocus()
        */
        //emuImage.image?.draw(in: NSRect(x: 0, y: 0, width: 240, height: 256))
        emuView.setNeedsDisplay(NSRect(x: 0, y: 0, width: 240, height: 256))
        //
        
        //CFRunLoopRun()
        //emuView.draw()
    }
    
}
