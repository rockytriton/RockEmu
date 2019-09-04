//
//  EmuView.swift
//  RockEmu
//
//  Created by Rocky Pulley on 8/29/19.
//  Copyright © 2019 Rocky Pulley. All rights reserved.
//

import Cocoa
import IOKit
import IOKit.usb
import IOKit.hid

class EmuView: NSView {
    @IBOutlet weak var imgView: NSImageView!
    
    public struct PixelData {
        var a:UInt8 = 255
        var r:UInt8
        var g:UInt8
        var b:UInt8
    }
    
    private let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
    private let bitmapInfo:CGBitmapInfo = CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue)
    
    public static var drawing = false
    
    var times = 0;
    var ppuData : UnsafeMutablePointer<PpuData>
    
    var pixelData = [PixelData](repeating: PixelData(a:0,r:0,g:0,b:0), count: 240 * 256);
    
    var imageData = [[Int?]](
        repeating: [Int?](repeating: nil, count: 256),
        count: 240
    )
    
    required init?(coder decoder: NSCoder) {
        
        ppuData = ppu_data_pointer();
        super.init(coder: decoder)
        
        for y in 0..<240 {
            for x in 0..<256 {
                imageData[y][x] = 0xffffffff
            }
        }
    }
    
    override var acceptsFirstResponder: Bool {
        return true
    }
    
    func drawPatterns() {
        var line = 0
        var offset = 0
        for two in 0..<16 {
            for one in 0..<16 {
                for y in 0..<8 {
                    for x in 0..<8 {
                        var n = ppu_get_pattern(UInt16(one + (two * 16)), UInt8(x), UInt8(y))
                        
                        
                        if (n == 0) {
                            imageData[line + (8 * two)][x + offset] = 0
                            //bytes[x + (offset) + ((240 * 255) - ((240 * line) + (240 * 8 * two)))] = 0xff000fff
                        } else if (n == 1) {
                            imageData[line + (8 * two)][x + offset] = 0x00ff00ff
                            //bytes[x + (offset) + ((240 * 255) - ((240 * line) + (240 * 8 * two)))] = 0xffffffff
                        } else if (n == 2) {
                            imageData[line + (8 * two)][x + offset] = 0x0000ffff
                            //bytes[x + (offset) + ((240 * 255) - ((240 * line) + (240 * 8 * two)))] = 0xcccccccc
                        } else {
                            imageData[line + (8 * two)][x + offset] = 0xff0000ff
                            //bytes[x + (offset) + ((240 * 255) - ((240 * line) + (240 * 8 * two)))] = 0xffff0000
                            //return;
                        }
                    }
                    
                    line = line + 1
                }
                offset += 8
                line = 0
            }
            offset = 0
            //line = two
        }
    }
    
    public func imageFromARGB32Bitmap(pixels:[PixelData], width:Int, height:Int)->NSImage {
        let bitsPerComponent:Int = 8
        let bitsPerPixel:Int = 32
        
        assert(pixels.count == Int(width * height))
        
        var data = pixels // Copy to mutable []
        let providerRef = CGDataProvider(
            data: NSData(bytes: &data, length: data.count * MemoryLayout.size(ofValue: pixels[0]))
        )
        
        let cgim = CGImage(
            width: width,
            height: height,
            bitsPerComponent: bitsPerComponent,
            bitsPerPixel: bitsPerPixel,
            bytesPerRow: width * Int(MemoryLayout.size(ofValue: pixels[0])),
            space: rgbColorSpace,
            bitmapInfo: bitmapInfo,
            provider: providerRef!,
            decode: nil,
            shouldInterpolate: true,
            intent: .defaultIntent
        )
        
        return NSImage(cgImage: cgim!, size: NSZeroSize)
    }
    
    var lastFrame : UInt32 = 0;
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        //ppu_get_pattern(<#T##n: UInt16##UInt16#>, <#T##x: UInt8##UInt8#>, <#T##y: UInt8##UInt8#>)
        
        if (!EmuView.drawing) {
            return;
        }
        
        if (ppuData.pointee.curFrame == lastFrame) {
            return;
        }
        
        lastFrame = ppuData.pointee.curFrame
        
        //print("Drawing Frame: ", lastFrame)
        
        var pb = ppu_data().pictureBuffer;
        var i = 0;
        
        for yyy in 0..<240 {
            for xxx in 0..<256 {
                //print("y,x = ", yyy, ",", xxx)
                var bt = pb![xxx]
                var n = Int(bt![yyy]);
                //imageData[yyy][xxx] = n
                
                var p = PixelData(a:255, r: UInt8((n & 0xFF000000) >> 24), g: UInt8((n & 0x00FF0000) >> 16), b: UInt8((n & 0x0000FF00) >> 8))
                pixelData[(yyy * 256) + xxx] = p;
            }
        }
        
        
        /*
        let image = NSImage(size: NSSize(width: 240, height: 256))
        
        image.lockFocus()
        
        for y in 0..<240 {
            for x in 0..<256 {
                let n = imageData[y][x]
                let scaledY = 255 - y
                let c = NSColor(red: CGFloat(((n! & 0xFF000000) >> 24)) / 255.0, green: CGFloat((n! & 0x00FF0000) >> 16) / 255.0, blue: CGFloat((n! & 0x0000FF00) >> 8) / 255.0, alpha: 1.0)
                c.drawSwatch(in: NSRect(x: x, y: scaledY, width: 1, height: 1))
            }
        }
        
        image.unlockFocus()
        */
        
        let image = imageFromARGB32Bitmap(pixels: pixelData, width: 256, height: 240)
        imgView.image = image
    }
    
    /*
     KEY CODE:  13
     KEY CODE:  1
     KEY CODE:  0
     KEY CODE:  2
     KEY CODE:  47
     KEY CODE:  44
     KEY CODE:  39
     KEY CODE:  36

 */
    
    var keyUp = false;
    var keyDown = false;
    var keyLeft = false;
    var keyRight = false;
    var keyA = false;
    var keyB = false;
    var keySel = false;
    var keyStart = false;
    
    override func keyUp(with event: NSEvent) {
        var kc = event.keyCode
        
        if (kc == 36) {
            controller_set(0, UInt8(BUTTON_ST), 0);
            keyStart = false;
        } else if (kc == 13) {
            controller_set(0, UInt8(BUTTON_U), 0);
            keyUp = false;
        } else if (kc == 1) {
            controller_set(0, UInt8(BUTTON_D), 0);
            keyDown = false;
        } else if (kc == 0) {
            controller_set(0, UInt8(BUTTON_L), 0);
            keyLeft = false;
        } else if (kc == 2) {
            controller_set(0, UInt8(BUTTON_R), 0);
            keyRight = false;
        } else if (kc == 47) {
            controller_set(0, UInt8(BUTTON_B), 0);
            keyB = false;
        } else if (kc == 44) {
            controller_set(0, UInt8(BUTTON_A), 0);
            keyA = false;
        } else if (kc == 39) {
            controller_set(0, UInt8(BUTTON_SE), 0);
            keySel = false;
        }
    }
    
    override func keyDown(with event: NSEvent) {
        var kc = event.keyCode
        
        //print("KEY CODE: ", kc);
        
        if (!keyStart && kc == 36) {
            controller_set(0, UInt8(BUTTON_ST), 1);
            keyStart = true
        } else if (!keyUp && kc == 13) {
            controller_set(0, UInt8(BUTTON_U), 1);
            keyUp = true
        } else if (!keyDown && kc == 1) {
            controller_set(0, UInt8(BUTTON_D), 1);
            keyDown = true
        } else if (!keyLeft && kc == 0) {
            controller_set(0, UInt8(BUTTON_L), 1);
            keyLeft = true
        } else if (!keyRight && kc == 2) {
            controller_set(0, UInt8(BUTTON_R), 1);
            keyRight = true
        } else if (!keyB && kc == 47) {
            controller_set(0, UInt8(BUTTON_B), 1);
            keyB = true
        } else if (!keyA && kc == 44) {
            controller_set(0, UInt8(BUTTON_A), 1);
            keyA = true
        } else if (!keySel && kc == 39) {
            controller_set(0, UInt8(BUTTON_SE), 1);
            keySel = true
            //ppu_dump_ram();
        }
    }
    
}
