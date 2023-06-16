gcc -I /Library/Frameworks/PicoSDK.framework/Headers /Library/Frameworks/PicoSDK.framework/Libraries/libps2000/libps2000.dylib -target x86_64-apple-macos11 -o ps2000Con ps2000Con.c

g++ -I /Library/Frameworks/PicoSDK.framework/Headers /Library/Frameworks/PicoSDK.framework/Libraries/libps2000/libps2000.dylib -target x86_64-apple-macos11 -o dd minirecorder2.cpp