/* 
 Boxer is copyright 2009 Alun Bestor and contributors.
 Boxer is released under the GNU General Public License 2.0. A full copy of this license can be
 found in this XCode project at Resources/English.lproj/GNU General Public License.txt, or read
 online at [http://www.gnu.org/licenses/gpl-2.0.txt].
 */


//BXEmulator is our many-tentacled Cocoa wrapper for DOSBox's low-level emulation functions.
//BXEmulator itself exposes an API for managing emulator startup, shutdown and general state.
//It is extended by more specific categories for managing more other aspects of emulator functionality.

//Because they talk directly to DOSBox, BXEmulator and its categories are Objective C++. All calls
//to DOSBox emulation functionality pass through here or one of its categories.

//Instances of this class are created by BXSession, and like BXSession the active emulator can be accessed
//as a singleton: via [[[NSApp delegate] currentSession] emulator] or just [BXEmulator currentEmulator].


#import <Foundation/Foundation.h>

#pragma mark -
#pragma mark Emulator constants

enum {
	BXSpeedFixed	= NO,
	BXSpeedAuto		= YES
};

enum {
	BXCoreUnknown	= -1,
	BXCoreNormal	= 0,
	BXCoreDynamic	= 1,
	BXCoreSimple	= 2,
	BXCoreFull		= 3
};

typedef BOOL BXSpeedMode;
typedef NSInteger BXCoreMode;


//C string encodings, used by BXShell executeCommand:encoding: and executeCommand:withArgumentString:encoding:
extern NSStringEncoding BXDisplayStringEncoding;	//Used for strings that will be displayed to the user
extern NSStringEncoding BXDirectStringEncoding;		//Used for file path strings that must be preserved raw


@class BXInputHandler;
@class BXVideoHandler;
@class BXGameProfile;

@protocol BXEmulatorDelegate;

@interface BXEmulator : NSObject
{
	id <BXEmulatorDelegate> delegate;
	BXInputHandler *inputHandler;
	BXVideoHandler *videoHandler;
	BXGameProfile *gameProfile;
	
	NSString *processName;
	NSString *processPath;
	NSString *processLocalPath;
	
	NSMutableArray *configFiles;
	NSMutableDictionary *driveCache;
	
	BOOL cancelled;
	BOOL executing;
	BOOL isInterrupted;
	
	//Used by BXShell
	NSMutableArray *commandQueue;
}


#pragma mark -
#pragma mark Properties

//Whether the emulator is currently running/cancelled respectively. Mirrors interface of NSOperation.
//The setters are for internal use only and should not be called outside of BXEmulator.
@property (assign, getter=isExecuting) BOOL executing;
@property (assign, getter=isCancelled) BOOL cancelled;

//The delegate responsible for this emulator.
@property (assign) id <BXEmulatorDelegate> delegate;

//The game profile we should refer to for tweaking emulation rules.
@property (retain) BXGameProfile *gameProfile;

//The name of the currently-executing DOSBox process. Will be nil if no process is running.
@property (copy) NSString *processName;

//The DOS filesystem path of the currently-executing DOSBox process.
//Will be nil if no process is running.
@property (copy) NSString *processPath;

//The local filesystem path of the currently-executing DOSBox process.
//Will be nil if no process is running or if the process is on an image or DOSBox-internal drive.
@property (copy) NSString *processLocalPath;

@property (readonly) BXInputHandler *inputHandler;	//Our DOSBox input handler.
@property (readonly) BXVideoHandler *videoHandler;	//Our DOSBox video and rendering handler.

//An array of OS X paths to configuration files that will be/have been loaded by this session during startup.
//This is read-only: configuration files can be loaded via applyConfigurationAtPath: 
@property (readonly) NSArray *configFiles;

//An array of queued command strings to execute on the DOS command line.
@property (readonly) NSMutableArray *commandQueue;

@property (assign) NSInteger fixedSpeed;	//The current fixed CPU speed.
@property (assign, getter=isAutoSpeed) BOOL autoSpeed;	//Whether we are running at automatic maximum speed.
@property (assign) BXCoreMode coreMode;		//The current CPU core mode.


#pragma mark -
#pragma mark Class methods

//Returns the currently active DOS session.
+ (BXEmulator *) currentEmulator;

//An array of names of internal DOSBox processes.
+ (NSArray *) internalProcessNames;

//Returns whether the specified process name is a DOSBox internal process (according to internalProcessNames).
+ (BOOL) isInternal: (NSString *)processName;


#pragma mark -
#pragma mark Controlling emulation state

//Begin emulation.
- (void) start;

//Stop emulation.
- (void) cancel;

//Load the DOSBox configuration file at the specified path.
//Currently, this only takes effect if done before [BXEmulator start] is called.
- (void) applyConfigurationAtPath: (NSString *)configPath;


#pragma mark -
#pragma mark Introspecting emulation state

//Returns whether DOSBox is currently running a process.
- (BOOL) isRunningProcess;

//Returns whether the current process (if any) is an internal process.
- (BOOL) processIsInternal;

//Returns whether DOSBox is currently inside a batch script.
- (BOOL) isInBatchScript;

//Returns whether DOSBox is waiting patiently at the DOS prompt doing nothing.
- (BOOL) isAtPrompt;


#pragma mark -
#pragma mark Responding to application state

//Used to notify the emulator that it will be interrupted by UI events.
//This will mute sound and otherwise prepare DOSBox for pausing.
- (void) willPause;
- (void) didResume;

@end


#if __cplusplus

class DOS_Shell;

//The methods in this category should not be executed outside of BXEmulator categories
//or BXCoalface functions, and are only visible in Objective C++.
@interface BXEmulator (BXEmulatorInternals)

- (DOS_Shell *) _currentShell;


//Called during DOSBox's event handling function: returns YES to abort event handling
//for that loop or NO to continue it.
- (BOOL) _handleEventLoop;

//Called during DOSBox's run loop: returns YES to short-circuit the loop.
- (BOOL) _handleRunLoop;

//Called at emulator startup.
- (void) _startDOSBox;

//Shortcut method for sending a notification both to the default notification center
//and to a selector on our delegate. The object of the notification will be self.
- (void) _postNotificationName: (NSString *)name
			  delegateSelector: (SEL)selector
					  userInfo: (id)userInfo;

//Called by DOSBox whenever it changes states we care about. This resyncs BXEmulator's
//cached notions of the DOSBox state, and posts notifications for relevant properties.
- (void) _didChangeEmulationState;

@end

#endif