import getopt, platform, sys, os, socket

remoteBuildFilesDir=""
remotePlatform="windows"
remoteAddr = os.environ["G3D10_REMOTE_MACHINE"]

    
s = platform.system()
windows = (s == 'Windows') or (s == 'Microsoft')
freebsd = (s == 'FreeBSD')
linux   = (s == 'Linux')
osx     = (s == 'Darwin')
cygwin  = s.lower().startswith('cygwin')
unix    = freebsd or linux or osx or cygwin


try:
    opts, args = getopt.getopt(sys.argv[1:], "r:l:d:h", ["remotePlatform=", "localBuildFilesDir=", "remoteBuildFilesDir=", "--help"])
    
except Exception as e:
    print("Error getting options: ")
    print(e)
    exit(1)

for opt, arg in opts:
    if opt in ['-r', '--remotePlatform']:
        remotePlatform = arg
    if opt in ['-d', '--remoteBuildFilesDir']:
        remoteBuildFilesDir = arg
    if opt in ['-h', '--help']:
        print(
"""
InstantLaunch Prebuild Script: 
-r  --remotePlatform        the remote platform
-d  --remoteBuildFilesDir   where the remote build files are located
-h  --help                  these instructions
"""
        )

if not remoteBuildFilesDir:
    print(
"""
No build file directory was passed in...
Please use either -d or --remoteBuildFilesDir to pass in the argument
aborting...
"""
)
    exit(1)

def sanitizePath(path):
    splitString = path.replace('\\', '/').split(':')
    return '/mnt/' + splitString[0].lower() + splitString[1]

print("Starting clean up script...")

if windows:
    localHostname = socket.gethostname()
    if localHostname in remoteAddr or "localhost" in remoteAddr:
        print("Remote Machine is the same as local... performing local cleanup...")
        remoteBuildFilesDir = sanitizePath(remoteBuildFilesDir)
        tempFileOutput = os.popen("wsl cat {}/temp.txt".format(remoteBuildFilesDir)).read()
        print("Processes to be terminated:\n", tempFileOutput)

        print("Terminating processes...")
        for process in tempFileOutput.splitlines():
            terminateOutput = os.popen("taskkill /fi \"ImageName eq {}\"".format(process)).read()
            print(terminateOutput)
        
        print("removing temp file... ")
        removeTempFileOutput = os.popen("wsl rm {}/temp.txt".format(remoteBuildFilesDir)).read()


    elif remotePlatform == "windows":
        remoteBuildFilesDir = sanitizePath(remoteBuildFilesDir)
        tempFileOutput = os.popen("wsl ssh %G3D10_REMOTE_MACHINE% wsl cat {}/temp.txt".format(remoteBuildFilesDir)).read()
        print("Processes to be terminated:\n", tempFileOutput)

        print("Terminating processes...")
        for process in tempFileOutput.splitlines():
            terminateOutput = os.popen("wsl ssh %G3D10_REMOTE_MACHINE% taskkill /fi \"ImageName eq {}\"".format(process)).read()
            print(terminateOutput)
        
        print("removing temp file... ")
        removeTempFileOutput = os.popen("wsl ssh %G3D10_REMOTE_MACHINE% wsl rm {}/temp.txt".format(remoteBuildFilesDir)).read()

    # TODO update for case that matchmaker is also remote
    print("Terminating matchmaker on local machine...")
    terminateMatchmaker = os.popen("taskkill /fi \"ImageName eq matchmaker.exe\"").read()
    print(terminateMatchmaker)

print("Clean up script done!")